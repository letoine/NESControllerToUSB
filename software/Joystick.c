
#include "Joystick.h"

static uint8_t PrevJoystickHIDReportBuffer[sizeof(USB_JoystickReport_Data_t)];

USB_ClassInfo_HID_Device_t Joystick_HID_Interface = {
  .Config = {
    .InterfaceNumber = INTERFACE_ID_Joystick,
    .ReportINEndpoint = {
      .Address              = JOYSTICK_EPADDR,
      .Size                 = JOYSTICK_EPSIZE,
      .Banks                = 1,
    },
    .PrevReportINBuffer           = PrevJoystickHIDReportBuffer,
    .PrevReportINBufferSize       = sizeof(PrevJoystickHIDReportBuffer),
  },
};

#define JST_PORT	PORTB
#define CLOCK_PIN	0
#define LATCH_PIN	1
#define DATA1_PIN	2
#define DATA2_PIN	3
#define DATA3_PIN	4

inline void set_clock_high() {
  JST_PORT |= _BV(CLOCK_PIN);
}

inline void set_clock_low() {
  JST_PORT &= ~(_BV(CLOCK_PIN));
}

inline void set_latch_high() {
  JST_PORT |= _BV(LATCH_PIN);
}

inline void set_latch_low() {
  JST_PORT &= ~(_BV(LATCH_PIN));
}

inline bool is_data1_low() {
  return !(PINB & _BV(DATA1_PIN));
}

inline bool is_data2_low() {
  return !(PINB & _BV(DATA2_PIN));
}

inline bool is_data3_low() {
  return !(PINB & _BV(DATA3_PIN));
}

inline void toggle_clock() {
  set_clock_high();
  _delay_ms(1);
  set_clock_low();
}

int main(void) {
  SetupHardware();
  
  GlobalInterruptEnable();
  
  for (;;) {
    HID_Device_USBTask(&Joystick_HID_Interface);
    USB_USBTask();
  }
}

void SetupHardware(void) {
  /* Disable watchdog if enabled by bootloader/fuses */
  MCUSR &= ~(1 << WDRF);
  wdt_disable();
  
  /* Disable clock division */
  clock_prescale_set(clock_div_1);
  
  /* Hardware Initialization */
  /* Port B pins for clock and latch are set to output */
  DDRB = (_BV(CLOCK_PIN)|_BV(LATCH_PIN));
  /* Set pull-up ressitors for inputs */
  PORTB = (_BV(DATA1_PIN)|_BV(DATA2_PIN)|_BV(DATA3_PIN));
  
  SerialDebug_init();
  USB_Init();
}

void EVENT_USB_Device_ConfigurationChanged(void) {
  bool ConfigSuccess = true;
  
  SerialDebug_printf("EVENT_USB_Device_ConfigurationChanged\r\n");
  ConfigSuccess &= HID_Device_ConfigureEndpoints(&Joystick_HID_Interface);
  
  USB_Device_EnableSOFEvents();
}

void EVENT_USB_Device_ControlRequest(void) {
  SerialDebug_printf("EVENT_USB_Device_ControlRequest\r\n");
  HID_Device_ProcessControlRequest(&Joystick_HID_Interface);
}

void EVENT_USB_Device_StartOfFrame(void) {
  //SerialDebug_printf("EVENT_USB_Device_StartOfFrame\r\n");
  HID_Device_MillisecondElapsed(&Joystick_HID_Interface);
}

bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
  USB_JoystickReport_Data_t* JoystickReport = (USB_JoystickReport_Data_t*)ReportData;
  
  set_clock_high();
  set_latch_high();

  JoystickReport->Y = 0;
  JoystickReport->X = 0;
  JoystickReport->button.raw[0] = 0;
  JoystickReport->button.raw[1] = 0;
  
  set_latch_low();
  
  toggle_clock();
  if (is_data1_low()) // B
    JoystickReport->button.cross = 1;
  
  toggle_clock();
  if (is_data1_low()) // Y
    JoystickReport->button.square =  1;
  
  toggle_clock();
  if (is_data1_low()) // Select
    JoystickReport->button.select = 1;
  
  toggle_clock();
  if (is_data1_low()) // Start
    JoystickReport->button.start = 1;
  
  toggle_clock();
  if (is_data1_low()) // up
    JoystickReport->Y = -127;
  
  toggle_clock();
  if (is_data1_low()) // down
    JoystickReport->Y = 127;
  
  toggle_clock();
  if (is_data1_low()) // left
    JoystickReport->X = -127;
  
  toggle_clock();
  if (is_data1_low()) // right
    JoystickReport->X = 127;
  
  toggle_clock();
  if (is_data1_low()) // A
    JoystickReport->button.circle = 1;
  
  toggle_clock();
  if (is_data1_low()) // X
    JoystickReport->button.triangle = 1;
  
  toggle_clock();
  if (is_data1_low()) // L
    JoystickReport->button.l1 = 1;
  
  toggle_clock();
  if (is_data1_low()) // R
    JoystickReport->button.r1 = 1;
  
  toggle_clock();
  toggle_clock();
  toggle_clock();
  _delay_ms(6);
  set_clock_high();
  
  *ReportSize = sizeof(USB_JoystickReport_Data_t);
  return false;
}

void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
  SerialDebug_printf("CALLBACK_HID_Device_ProcessHIDReport\r\n");
  SerialDebug_printf("%#0x, %#0x\r\n", ReportID, ReportType);
  SerialDebug_printByteArray(ReportData, ReportSize);
  
  // Unused (but mandatory for the HID class driver) in this demo, since there are no Host->Device reports
}
