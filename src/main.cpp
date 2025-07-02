#include <Arduino.h>
#include <SimpleFOC.h>
#include "SimpleCAN.h"
#include "CANProfile.h"

// #define LED_PIN PC6
#define BUTTON_PIN PC10

// Motor instance
BLDCMotor motor = BLDCMotor(7);
BLDCDriver6PWM driver = BLDCDriver6PWM(A_PHASE_UH, A_PHASE_UL, A_PHASE_VH, A_PHASE_VL, A_PHASE_WH, A_PHASE_WL);

float target = 1.0;
void serialLoop() {
  static String received_chars;

  while (Serial.available()) {
    char inChar = (char) Serial.read();
    received_chars += inChar;
    if (inChar == '\n') {
      target = received_chars.toFloat();
      Serial.print("Target = "); Serial.println(target);
      received_chars = "";
    }
  }
}

class RxFromCAN : public NotificationsFromCAN
{
public:
  RxFromCAN() : ReceivedID(-1), RTR(false) , ReceivedFloatVal(1.0f){};

  void ReceivedFloat(const int Device, const float Val)
  {
    Serial.printf("Rcvd float: %.3f from 0x%x\n", Val, Device);
    ReceivedFloatVal = Val;
    ReceivedID = CANID_FLOAT;
  };

		// void ReceivedRequestInt(const int Device)
		// {
		// 	Serial.printf("Received: RTR from 0x%x\n", Device);
		// 	ReceivedID = CANID_RTRINT;
		// 	RTR = true;
		// };

		// void ReceivedInt(const int Device, int Val)
		// {
		// 	Serial.printf("Rcvd int: %d from 0x%x\n", Val, Device);
		// 	ReceivedID = CANID_RTRINT;
		// };

		int ReceivedID;		
		bool RTR = true;
		float ReceivedFloatVal;
};

RxFromCAN CANBroker;

CANProfile CANDevice(CreateCanLib(A_CAN_TX, A_CAN_RX), &CANBroker);

int deviceID=0;

void setup() {
  
  // use monitoring with serial 
  Serial.begin(115200);
  // enable more verbose output for debugging
  // comment out if not needed
  // SimpleFOCDebug::enable(&Serial);
  
  // driver config
  // power supply voltage [V]
  driver.voltage_power_supply = 12;
  driver.pwm_frequency = 30000;
  driver.init();
  // link the motor and the driver
  motor.linkDriver(&driver);

  // set motion control loop to be used
  motor.controller = MotionControlType::velocity_openloop;

  motor.PID_velocity.P = 0.2;
  motor.PID_velocity.I = 20;

  // jerk control using voltage voltage ramp
  // default value is 300 volts per sec  ~ 0.3V per millisecond
  motor.PID_velocity.output_ramp = 1000;
 
  // velocity low pass filtering time constant
  motor.LPF_velocity.Tf = 0.01;

  // angle P controller
  motor.P_angle.P = 20;

  motor.voltage_limit = 12;
 
  motor.velocity_limit = 4;

  // comment out if not needed
  // motor.useMonitoring(Serial);
  
  // initialize motor
  motor.init();
  // align encoder and start FOC
  motor.initFOC();

  Serial.println(F("Motor ready."));
  Serial.println(F("Set the target angle using serial terminal:"));
  _delay(1000);

  /****************************************
   * Set up CAN
   ****************************************/
  pinMode(A_CAN_TERM, OUTPUT);
  digitalWrite(A_CAN_TERM, HIGH);
  deviceID = 0x21; 

  // // Note: Blinking will only work if LED_BUILTIN is defined and the board supports it!
	// CANDevice.Can1->EnableBlinkOnActivity(LED_BUILTIN);

  Serial.printf("Setup done, random device ID is %d (0x%x)\n", deviceID, deviceID);
}

void loop() {
  // Serial.println("yeeee");
  serialLoop();
  // main FOC algorithm function
  // motor.loopFOC();

  // // Motion control function
  // motor.move(target);

  // function intended to be used with serial plotter to monitor motor variables
  // significantly slowing the execution down!!!!
  // motor.monitor();

  /*********************************
  CAN implementation
  **********************************/

  Serial.printf("Sending: %.3f\n", target);
  CANDevice.CANSendFloat(target, MAKE_CAN_ID(deviceID, CANID_FLOAT));

  // Get some statistics on bus errors.
	static int LastTxErrors=0;
	static int LastRxErrors=0;
	static int LastOtherErrors = 0;
	static uint32_t LastStatus = 0;
	uint32_t Status = 0;
	char StatusStr[MAX_STATUS_STR_LEN]={0};

  CANDevice.Can1->GetStatus(&Status, StatusStr);
	if (CANDevice.Can1->GetTxErrors()!=LastTxErrors || CANDevice.Can1->GetRxErrors()!=LastRxErrors || CANDevice.Can1->GetOtherErrors()!=LastOtherErrors || LastStatus!=Status)
	{
		LastTxErrors = CANDevice.Can1->GetTxErrors();
		LastRxErrors = CANDevice.Can1->GetRxErrors();
		LastOtherErrors = CANDevice.Can1->GetOtherErrors();
		LastStatus = Status;

		Serial.printf("New Status=%s, RxErrors=%d, TxErrors=%d, Other=%d\n", StatusStr, LastTxErrors, LastRxErrors, LastOtherErrors);
	}

  delay(3000);

  // Update message queues.
  CANDevice.Can1->Loop();
}
