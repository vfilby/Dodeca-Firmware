// Magpie/Neutron Dodeca Test 
//
// This script will iterate through CC values 0, 32, 64, 96 and 128 yielding 
// approximately 0v, 2.5v, 5v, 7.50v and 10v on the output CV jacks.  You can 
// use this in conjuntion with a module that responds predictably to CV
// voltage or a voltmeter (or DMM) to test the output values.
//
// The script will pause for 2s between each voltage and 5s at the end to 
// give you time to switch jacks.  Values being set are also written to the 
// serial port so you can double check what is being set.
//
// v@filby.ca


int output_count = 12;

// top to bottom, left to right (order only affects startup sequence)
//int output_pin[] = {23,A14,22,25,20,6,21,5,9,4,10,3};

// left to right, bottom to top (order only affects startup sequence)
int output_pin[] = {10,9,21,20,22,23,A14,25,6,5,4,3};


void setup() {
  Serial.begin(9600);
     
  for( int i = 0; i < output_count; i++ ) {
    
    // Not quite sure why this is skipped, but the examples omitted the DAC pin.
    if( output_pin[i] == A14 )
      continue;

    Serial.println( String( "Configuring pin " ) + output_pin[i] );
    pinMode( output_pin[i], OUTPUT );
    analogWriteFrequency( output_pin[i], 40000 );
  }

  analogWriteResolution(12);

  // Startup LED sequence
  for (int i = 0;i < output_count; i++ ){
    analogWrite(output_pin[i], 128 << 5 );
    delay(50);
    analogWrite(output_pin[i], 0);
  }

}

void loop() {
  for( int i = 0; i < 129; i += 32) {
    Serial.println(String( "CC Value=" ) + i + ", Scaled CC Value=" + (i << 5) + ", ~V=" + (i/128.0*10.0) );
    for( int j = 0; j < 12; j++ ) {
      analogWrite( output_pin[j], (i << 5) );
    }
    delay( 2000 );
  }
  Serial.println( "Try a different jack" );
  delay( 5000 );
}
