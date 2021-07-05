// A generic firmware for the Magpie/Neutron Dodeca Module that supports Gate, Pitch (with Bend), Velocity and the 9 remaining jacks are 
// all generic CV from 1 to n. 
//
// Original basis for this is https://github.com/jakplugg/Dodeca/blob/master/Neutron_magpie_dodeca_MIDI_OUT/_12_CV_from_CC/_12_CV_from_CC.ino 
// and many parts are copied verbatim as I didn't need to dig into some of the logic.
//
// Some notable fixes from the original:
// * Note off tracks current note so releasing a previously held note doesn't kill the current one
// * Added pitch bending
//
// v@filby.ca

#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

const int GATE_PIN = 0;
const int NOTE_PIN = 1;
const int VELOCITY_PIN = 2;

int output_count = 12;
int output_pin[] = {23,A14,22,25,20,6,21,5,9,4,10,3};
int bottomNote = 12; //the bottom MIDI note, the module puts out 8 octaves.  
int topNote = bottomNote + 95;
int pitch_scaled;
int cc2out[] = {40,41,42,1,2,3,4,5,6,7,8,9}; //ignore first 3 these are the MIDI CCs that will be output.
int current_pitch = -1;
float current_bend = 0;

const int SERIAL_BAUD = 9600;

const int SCALED_OCTAVE_INTERVAL = 432;
const int PITCH_BEND_AMOUNT = 2; // in semitones.

// Math taken from 
int _pitchToAnalog( int note ) {
  return ((constrain( current_pitch, bottomNote, topNote ) - bottomNote) * 36);
}


void _outputPitch( int pitch, float bend ) {
  analogWrite( output_pin[NOTE_PIN], pitch + bend );
}


// To get the pitch bend we calculate the proportion of the maximum bend (default 2 ST)
// then multiply that by the theoretical(?) analog output of 2ST.  I grabbed the analog 
// measurements off the serial port, so they might not be universal.  YMMV.
//
void HandlePitchBend (byte channel, int bend){
  float max_bend = (SCALED_OCTAVE_INTERVAL/12) * PITCH_BEND_AMOUNT;
  float scaled_bend = (float)bend / MIDI_PITCHBEND_MAX;
  float delta = scaled_bend * max_bend;
  current_bend = delta;
  
  _outputPitch( _pitchToAnalog(current_pitch), current_bend );

  // Uncomment to Debug
  // Serial.println( String("PitchBend bend=") + bend + ", scaled_bend=" + scaled_bend + ", delta=" + delta + ", current_pitch=" + current_pitch  );
}

void HandleControlChange (byte channel, byte number, byte value){
  
  // for loop used for an array search, better ways of handling this. fix later.
  for (int i = 3; i < 12; i++) {
    if (number == cc2out[i]){ 
      analogWrite(output_pin[i],value<<5);    
    }
  } 
}

void HandleNoteOff(byte channel, byte pitch, byte velocity) {

  // Uncomment to Debug
  // Serial.println( String("Note off: ch=") + channel + ", pitch=" + pitch + ", velocity=" + velocity + ", current_pitch=" + current_pitch );
  
  // if multiple notes are held down only the last is played, stopping earlier notes shouldn't
  // affect any notes currently held down.
  if( pitch != current_pitch ) 
    return;
    
  digitalWrite( output_pin[GATE_PIN], 0 );
  current_pitch = -1;
}


void HandleNoteOn(byte channel, byte pitch, byte velocity) { 

  current_pitch = pitch;
  
  // Handle case where vel=0 is used to represent note off.
  if (velocity == 0) {
    HandleNoteOff( channel, pitch, velocity );
  }

  digitalWrite( output_pin[GATE_PIN], HIGH );
  analogWrite( output_pin[VELOCITY_PIN], velocity << 5 );    
  
  pitch_scaled = (constrain( pitch, bottomNote, topNote ) - bottomNote) * 36;
  _outputPitch( _pitchToAnalog(pitch_scaled), current_bend );
  
  // Uncomment to Debug
  // Serial.println(String("Note On: ch=") + channel + ", pitch=" + pitch + ", velocity=" + velocity + ", Scaled Velocity=" + (velocity<<5) + ", ScaledPitch=" + pitch_scaled );
}

void setup() {
  
  Serial.begin(SERIAL_BAUD);
  
  // Initialize the output pins on the teensy
  for( int i = 0; i < output_count; i++ ) {

    // Uncomment to Debug
    // Serial.println( String( "Configuring pin " ) + output_pin[i] );
    
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

  // Initiate MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI); 
  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandleControlChange(HandleControlChange); 
  MIDI.setHandleNoteOff(HandleNoteOff);
  MIDI.setHandlePitchBend( HandlePitchBend );

}

void loop() {
  // Call MIDI.read the fastest you can for real-time performance.
  MIDI.read();
}
