volatile int sensorReading, signalMax, signalMin; 
volatile boolean adcDone; // check if adc sample is available
float spl; // sound pressure level
boolean adcStarted, samplingOver;

int sampleWindow = 0.105; // sample windows width in mS (NOT ADC SAMPLE FREQUENCY!)
unsigned long startSample, startBuzzer;

// datasheet values
const double micSensitivity = -44, splReference = 0.00631, pascalReference = 94; 
const int  micGain = 25; 
int splThreshold[5] = {40, 50, 60, 70, 80}; // stores  SPL threshold values

void setup() 
{
   Serial.begin(115200);
   pinMode(6, OUTPUT);  //green
   pinMode(10, OUTPUT); //green
   pinMode(11, OUTPUT); //green
   pinMode(12, OUTPUT); //green
   pinMode(8, OUTPUT);  //yellow
   pinMode(9, OUTPUT);  //red
   pinMode(13, OUTPUT);  //active buzzer
   spl = 0;
   signalMax = 0;
   signalMin = 1023;
   adcDone = false;
   adcStarted = false;
   samplingOver = false;
   startSample = millis();

   ADCSRA = bit (ADEN); // enable ADC
   ADCSRA |= bit (ADPS0) |bit (ADPS1) | bit (ADPS2);  // prescaler of 128
   ADMUX = bit (REFS0) | (0 & 0x07); // read analog pin 0
}

// Interrupt triggered when ADC reading available
ISR (ADC_vect){
  sensorReading = ADC;// fetch the ADC reading
  if (sensorReading > signalMax){
    signalMax = sensorReading;
  }
  if (sensorReading < signalMin){
    signalMin = sensorReading;
  }
  adcDone = true; 
}
 
void loop()
{
  // process last reading
 if (adcDone == true){
   adcStarted = false; 
   adcDone = false;
   
   spl = calculateSpl(signalMin, signalMax);
   }

 // start a new reading
 if (adcStarted == false){
   adcStarted = true;
   // start ADC conversion and enable ISR for it
   ADCSRA |= bit (ADSC) | bit (ADIE);
 }

 if (spl != 0){
     ledTrigger(spl, splThreshold);
     buzzerTrigger(spl, splThreshold[5], 2000); // period in microseconds
 }
}


float calculateSpl(int minAdc, int maxAdc){
  //convert ADC value to dB SPL (RMS value)
  float peakToPeak, decibel, soundPressureLevel, rms;

  if((millis() - startSample) > sampleWindow){
    
    peakToPeak = maxAdc - minAdc;
    rms = ((peakToPeak * 3.3) / 1024.0) *0.707; //connected to pin with 3.3V
    decibel = 20 * log10(rms/splReference);  // reference level 1V
   
    soundPressureLevel = decibel + pascalReference + micSensitivity - micGain;

    Serial.print((String)"Voltage: " + rms);  
    Serial.print((String)"\tDecibel : " + decibel);
    Serial.print((String)"\tSPL : " + soundPressureLevel);
    Serial.print("\n");
    
    startSample = millis();
    return soundPressureLevel;
  }

  else {
    return 0;
  }
}

void buzzerTrigger(float value, int threshold, float period){ // delta=value-threshold [spl], period=self explaining [microseconds]
  float delta, onCycle, soundDuration;
  int rippleNumber, counter;
  
  delta = value - threshold;
  if ((delta > 1) & (value != 0)){
    onCycle = period * 0.5; // 50% duty cycle
    soundDuration = onCycle / delta;
    rippleNumber = period / soundDuration; 

    for(counter=0; counter<=rippleNumber; counter++){
      digitalWrite(13, HIGH);
      delay(soundDuration);
      digitalWrite(13, LOW);
      delay(soundDuration); 
  }
}
}

void ledTrigger(float value, int threshold[5]){ // number of on led, 1-4 green, 5 yellow, 6 red
  int newLedNum=0, ledNum=0; 
  
  if (spl <= threshold[0]){
    newLedNum = 1;
   }
   if ((spl > threshold[0]) && (spl <= threshold[1])){
    newLedNum = 2;
   }
   if ((spl > threshold[1]) && (spl <= threshold[2])){
    newLedNum = 3;
   }
   if ((spl > threshold[2]) && (spl <= threshold[3])){
    newLedNum = 4;
   }
   if ((spl > threshold[3]) && (spl <= threshold[4])){
    newLedNum = 5;
   }
   if (spl > threshold[5]){
    newLedNum = 6;
   }
   //update led
   if (newLedNum != ledNum){
    switch(newLedNum){
    case 1:
     digitalWrite(6, HIGH);
     digitalWrite(10, LOW);
     digitalWrite(11, LOW);
     digitalWrite(12, LOW);
     digitalWrite(8, LOW);
     digitalWrite(9, LOW);
     digitalWrite(13, LOW);
     break;
    case 2:
     digitalWrite(6, HIGH);
     digitalWrite(10, HIGH);
     digitalWrite(11, LOW);
     digitalWrite(12, LOW);
     digitalWrite(8, LOW);
     digitalWrite(9, LOW);
     digitalWrite(13, LOW);
     break;
    case 3:
     digitalWrite(6, HIGH);
     digitalWrite(10, HIGH);
     digitalWrite(11, HIGH);
     digitalWrite(12, LOW);
     digitalWrite(8, LOW);
     digitalWrite(9, LOW);
     digitalWrite(13, LOW); 
     break;
    case 4:
     digitalWrite(6, HIGH);
     digitalWrite(10, HIGH);
     digitalWrite(11, HIGH);
     digitalWrite(12, HIGH);
     digitalWrite(8, LOW);
     digitalWrite(9, LOW);
     digitalWrite(13, LOW);
     break;
    case 5: 
     digitalWrite(6, HIGH);
     digitalWrite(10, HIGH);
     digitalWrite(11, HIGH);
     digitalWrite(12, HIGH);
     digitalWrite(8, HIGH);
     digitalWrite(9, LOW);
     digitalWrite(13, LOW);
     break;
    case 6:
     digitalWrite(6, HIGH);
     digitalWrite(10, HIGH);
     digitalWrite(11, HIGH);
     digitalWrite(12, HIGH);
     digitalWrite(8, HIGH);
     digitalWrite(9, HIGH);
     digitalWrite(13, HIGH);
     break;
    case 0:
     digitalWrite(6, LOW);
     digitalWrite(10, LOW);
     digitalWrite(11, LOW);
     digitalWrite(12, LOW);
     digitalWrite(8, LOW);
     digitalWrite(9, LOW);
     digitalWrite(13, LOW);
     break; 
  }
  ledNum = newLedNum;
}
}
