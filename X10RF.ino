// ****************************************************************************
// |                        X10 RF transmitter                                |
// |                  Based on code by Lotfi BAGHLI                           |
// |         found here:  http://www.baghli.com/dspic_x10rf.php               |
// ****************************************************************************

#define A1_ON 0x6000 
#define A1_OFF 0x6020

volatile bool lightState = false;
volatile int sendCount = 0;

const int txPin = 6;
const int pwrPin = 5;
const int butPin = 2;
const int ledPin = 13;

int inbyte = 0;

//***************************************************************************--
void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);

	pinMode(butPin, INPUT_PULLUP);
	pinMode(txPin, OUTPUT);
	pinMode(pwrPin, OUTPUT);
	attachInterrupt(digitalPinToInterrupt(butPin), buttonPress, RISING);

	// Serial.println("Setup Complete");
}

//***************************************************************************--
void loop() {
	// put your main code here, to run repeatedly:

	// Check for some instructions on the serial com

	if (Serial.available()){
		inbyte = Serial.read();
		if (inbyte == 'A'){
			lightState = true;
			sendCount = 0;                // Reset the send count
		}
		else
		{
			lightState = false;
			sendCount = 0;                // Reset the send count
		}
	}

	//  If we should be sending instructions do it now

	if (sendCount < 3){
		//Serial.print("Sending - ");
		digitalWrite(ledPin, HIGH);
		if (lightState){
			SendX10Cmd(A1_ON);
			Serial.print("A1 On - ");
		}
		else {
			SendX10Cmd(A1_OFF);
			Serial.print("A1 Off - ");
		}
		//Serial.println (sendCount + 1, DEC);
		sendCount++;   // Advance the sendcount by 1
		delay(200);
	}
	else {
		digitalWrite(ledPin, LOW);
	}
}


//***************************************************************************--
void buttonPress(){
	// If the button is pressed do something here
	static unsigned long last_interrupt_time = 0;
	unsigned long interrupt_time = millis();
	// If interrupts come faster than 200ms, assume it's a bounce and ignore
	if (interrupt_time - last_interrupt_time > 200)
	{

		lightState = !lightState;     // Change the state
		sendCount = 0;                // Reset the send count
		last_interrupt_time = interrupt_time;
	}
}

//***************************************************************************--
void SendX10RF_bit(unsigned int databit)
{
	// If its a 1
	digitalWrite(txPin, HIGH);
	delayMicroseconds(560);    // 0.56 ms
	digitalWrite(txPin, LOW);
	delayMicroseconds(560);    // 0.56 ms
	// If its a 0
	if (!databit)  delayMicroseconds(560 * 2);  // 0.56*2 = 1.12 ms si un zero
}
//***************************************************************************--
void SendX10byte(unsigned int data8)
{
	unsigned int j, k;
	for (j = 0; j < 8; j++)
	{
		k = (data8 & 0x0080) == 0x0080;
		SendX10RF_bit(k);
		data8 = data8 << 1;
	}
}

//***************************************************************************--
void SendX10Cmd(unsigned int data)
{
	unsigned int Cmd8;
	digitalWrite(pwrPin, HIGH);      // Power On transmitter
	delay(20);  // >20 ms
	// en-tête
	digitalWrite(txPin, HIGH);
	delay(9);  // 8.96 ms
	digitalWrite(txPin, LOW);
	delayMicroseconds(4500);  // 4.5 ms
	Cmd8 = data >> 8;     // send les Hi
	SendX10byte(~Cmd8); // Send complement of data
	SendX10byte(Cmd8);
	Cmd8 = (data)& 0xFF;  // send les Low
	SendX10byte(~Cmd8);
	SendX10byte(Cmd8);
	SendX10RF_bit(1);   // 1 à la fin ??
	delayMicroseconds(112);    // 1.12 ms
	digitalWrite(pwrPin, HIGH);      // Power On transmitter
}
//***************************************************************************--

