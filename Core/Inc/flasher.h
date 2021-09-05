// flasher state variables
uint32_t *prog_ptr = NULL;
bool unlocked = false;
CAN_HandleTypeDef hcan1;
typedef union {
  uint16_t w;
  struct BW {
    uint8_t msb;
    uint8_t lsb;
  }
  bw;
}
uint16_t_uint8_t;

typedef union _USB_Setup {
  uint32_t d8[2];
  struct _SetupPkt_Struc
  {
    uint8_t           bmRequestType;
    uint8_t           bRequest;
    uint16_t_uint8_t  wValue;
    uint16_t_uint8_t  wIndex;
    uint16_t_uint8_t  wLength;
  } b;
}
USB_Setup_TypeDef;

bool flash_is_locked(void) {
  return (FLASH->CR & FLASH_CR_LOCK);
}

void flash_unlock(void) {
  FLASH->KEYR = 0x45670123;
  FLASH->KEYR = 0xCDEF89AB;
}

// TODO: add sector support
bool flash_erase_sector(uint8_t sector, bool unlocked) {
#if 0
  // don't erase the bootloader(sector 0)
  if (sector != 0 && sector < 12 && unlocked) {
    FLASH->CR = (sector << 3) | FLASH_CR_SER;
    FLASH->CR |= FLASH_CR_STRT;
    while (FLASH->SR & FLASH_SR_BSY);
    return true;
  }
#endif
  return false;
}

// TODO: add write
void flash_write_word(void *prog_ptr, uint32_t data) {
#if 0
  uint32_t *pp = prog_ptr;
  FLASH->CR = FLASH_CR_PSIZE_1 | FLASH_CR_PG;
  *pp = data;
  while (FLASH->SR & FLASH_SR_BSY);
#endif
}

void flush_write_buffer(void) { }

int usb_cb_control_msg(USB_Setup_TypeDef *setup, uint8_t *resp, bool hardwired) {
  int resp_len = 0;

  // flasher machine
  memset(resp, 0, 4);
  memcpy(resp+4, "\xde\xad\xd0\x0d", 4);
  resp[0] = 0xff;
  resp[2] = setup->b.bRequest;
  resp[3] = ~setup->b.bRequest;
  *((uint32_t **)&resp[8]) = prog_ptr;
  resp_len = 0xc;

  int sec;
  switch (setup->b.bRequest) {
    // **** 0xb0: flasher echo
    case 0xb0:
      resp[1] = 0xff;
      break;
    // **** 0xb1: unlock flash
    case 0xb1:
      if (flash_is_locked()) {
        flash_unlock();
        resp[1] = 0xff;
      }
      unlocked = true;
      prog_ptr = (uint32_t *)APP_START_ADDRESS;
      break;
    // **** 0xb2: erase sector
    case 0xb2:
      sec = setup->b.wValue.w;
      if (flash_erase_sector(sec, unlocked)) {
        resp[1] = 0xff;
      }
      break;
    // **** 0xd1: enter bootloader mode
    case 0xd1:
      // this allows reflashing of the bootstub
      // so it's blocked over wifi
      switch (setup->b.wValue.w) {
        case 0:
          // TODO: put this back when it's no longer a "devkit"
          enter_bootloader_mode = ENTER_BOOTLOADER_MAGIC;
          NVIC_SystemReset();
          break;
        case 1:
          enter_bootloader_mode = ENTER_SOFTLOADER_MAGIC;
          NVIC_SystemReset();
          break;
      }
      break;
    // **** 0xd8: reset ST
    case 0xd8:
      flush_write_buffer();
      NVIC_SystemReset();
      break;
  }
  return resp_len;
}

void usb_cb_ep2_out(void *usbdata, int len, bool hardwired) {
  UNUSED(hardwired);
  for (int i = 0; i < len/4; i++) {
    flash_write_word(prog_ptr, *(uint32_t*)(usbdata+(i*4)));
    //*(uint64_t*)(&spi_tx_buf[0x30+(i*4)]) = *prog_ptr;
    prog_ptr++;
  }
}


int spi_cb_rx(uint8_t *data, int len, uint8_t *data_out) {
  UNUSED(len);
  int resp_len = 0;
  switch (data[0]) {
    case 0:
      // control transfer
      resp_len = usb_cb_control_msg((USB_Setup_TypeDef *)(data+4), data_out, 0);
      break;
    case 2:
      // ep 2, flash!
      usb_cb_ep2_out(data+4, data[2], 0);
      break;
  }
  return resp_len;
}

#define CAN_BL_INPUT 0x1
#define CAN_BL_OUTPUT 0x2

void CAN1_TX_IRQHandler(void) {
  // clear interrupt
  CAN1->TSR |= CAN_TSR_RQCP0;
}

#define ISOTP_BUF_SIZE 0x110

uint8_t isotp_buf[ISOTP_BUF_SIZE];
uint8_t *isotp_buf_ptr = NULL;
int isotp_buf_remain = 0;

uint8_t isotp_buf_out[ISOTP_BUF_SIZE];
uint8_t *isotp_buf_out_ptr = NULL;
int isotp_buf_out_remain = 0;
int isotp_buf_out_idx = 0;

void bl_can_send(uint8_t *odat) {
  // wait for send
  while (!(CAN1->TSR & CAN_TSR_TME0));

  // send continue
  CAN1->sTxMailBox[0].TDLR = ((uint32_t*)odat)[0];
  CAN1->sTxMailBox[0].TDHR = ((uint32_t*)odat)[1];
  CAN1->sTxMailBox[0].TDTR = 8;
  CAN1->sTxMailBox[0].TIR = (CAN_BL_OUTPUT << 21) | 1;
}

void CAN1_RX0_IRQHandler(void) {
  while (CAN1->RF0R & CAN_RF0R_FMP0) {
    if ((CAN1->sFIFOMailBox[0].RIR>>21) == CAN_BL_INPUT) {
      uint8_t dat[8];
      for (int i = 0; i < 8; i++) {
        dat[i] = GET_BYTE(&CAN1->sFIFOMailBox[0], i);
      }
      uint8_t odat[8];
      uint8_t type = dat[0] & 0xF0;
      if (type == 0x30) {
        // continue
        while (isotp_buf_out_remain > 0) {
          // wait for send
          while (!(CAN1->TSR & CAN_TSR_TME0));

          odat[0] = 0x20 | isotp_buf_out_idx;
          memcpy(odat+1, isotp_buf_out_ptr, 7);
          isotp_buf_out_remain -= 7;
          isotp_buf_out_ptr += 7;
          isotp_buf_out_idx++;

          bl_can_send(odat);
        }
      } else if (type == 0x20) {
        if (isotp_buf_remain > 0) {
          memcpy(isotp_buf_ptr, dat+1, 7);
          isotp_buf_ptr += 7;
          isotp_buf_remain -= 7;
        }
        if (isotp_buf_remain <= 0) {
          int len = isotp_buf_ptr - isotp_buf + isotp_buf_remain;

          // call the function
          memset(isotp_buf_out, 0, ISOTP_BUF_SIZE);
          isotp_buf_out_remain = spi_cb_rx(isotp_buf, len, isotp_buf_out);
          isotp_buf_out_ptr = isotp_buf_out;
          isotp_buf_out_idx = 0;

          // send initial
          if (isotp_buf_out_remain <= 7) {
            odat[0] = isotp_buf_out_remain;
            memcpy(odat+1, isotp_buf_out_ptr, isotp_buf_out_remain);
          } else {
            odat[0] = 0x10 | (isotp_buf_out_remain>>8);
            odat[1] = isotp_buf_out_remain & 0xFF;
            memcpy(odat+2, isotp_buf_out_ptr, 6);
            isotp_buf_out_remain -= 6;
            isotp_buf_out_ptr += 6;
            isotp_buf_out_idx++;
          }

          bl_can_send(odat);
        }
      } else if (type == 0x10) {
        int len = ((dat[0]&0xF)<<8) | dat[1];

        // setup buffer
        isotp_buf_ptr = isotp_buf;
        memcpy(isotp_buf_ptr, dat+2, 6);

        if (len < (ISOTP_BUF_SIZE-0x10)) {
          isotp_buf_ptr += 6;
          isotp_buf_remain = len-6;
        }

        memset(odat, 0, 8);
        odat[0] = 0x30;
        bl_can_send(odat);
      }
    }
    // next
    CAN1->RF0R |= CAN_RF0R_RFOM0;
  }
}

void llcan_clear_send(CAN_TypeDef *CAN_obj) {
  SET_BIT(CAN_obj->TSR, CAN_TSR_ABRQ0);
  CLEAR_BIT(CAN_obj->MSR, CAN_MSR_ERRI);
  // cppcheck-suppress selfAssignment ; needed to clear the register
  CAN_obj->MSR = CAN_obj->MSR;
}

void CAN1_SCE_IRQHandler(void) {
  llcan_clear_send(CAN1);
}

static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 4;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_15TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

void soft_flasher_start(void) {

  MX_CAN1_Init();

  enter_bootloader_mode = 0;

  // TODO: Init flash peripherals
 
  /* Configure the CAN Filter */
  CAN_FilterTypeDef sFilterConfig1;
  sFilterConfig1.FilterBank = 14;
  sFilterConfig1.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig1.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig1.FilterIdHigh = 0x0000;
  sFilterConfig1.FilterIdLow = 0x0000;
  sFilterConfig1.FilterMaskIdHigh = 0x0000;
  sFilterConfig1.FilterMaskIdLow = 0x0000;
  sFilterConfig1.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig1.FilterActivation = ENABLE;
  sFilterConfig1.SlaveStartFilterBank = 14;

  if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig1) != HAL_OK)
  {
    /* Filter configuration Error */
    Error_Handler();
  }

  /* Start the CAN peripheral */
  if (HAL_CAN_Start(&hcan1) != HAL_OK)
  {
    /* Start Error */
    Error_Handler();
  }

  /* Activate CAN RX notification */
  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING |
              CAN_IT_RX_FIFO1_MSG_PENDING |
              CAN_IT_TX_MAILBOX_EMPTY |
              CAN_IT_ERROR |
              CAN_IT_LAST_ERROR_CODE |
              CAN_IT_ERROR_PASSIVE |
              CAN_IT_ERROR_WARNING) != HAL_OK)
  {
    /* Notification Error */
    Error_Handler();
  }

  enable_interrupts();
}

