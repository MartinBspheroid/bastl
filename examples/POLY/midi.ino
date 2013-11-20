

#define CONTROL_CHANGE_BITS 7
#define CONTROL_CHANGE_OFFSET 102

unsigned char midiSound;

#define POLYPHONY NUMBER_OF_VOICES
unsigned char notesInBuffer=ZERO;
boolean thereIsNoteToPlay;

#define BUFFER_SIZE 16
unsigned char midiBuffer[BUFFER_SIZE];
#define DEFAULT_VOICE_USE 255

unsigned char voiceUse[NUMBER_OF_VOICES]={
  DEFAULT_VOICE_USE,DEFAULT_VOICE_USE,DEFAULT_VOICE_USE};

unsigned char fromBuffer;
boolean ping;


void shiftBufferLeft(unsigned char from){
  for(int i=from;i<notesInBuffer;i++){
    midiBuffer[i]=midiBuffer[i+1]; 
  }
}

void shiftBufferRight(){
  for(int i=notesInBuffer;i>ZERO;i--){
    midiBuffer[i]=midiBuffer[i-1]; 
  }
}

boolean isThereNoteToPlay(){
  return thereIsNoteToPlay;

}
unsigned char noteToPlay(){
  thereIsNoteToPlay=false;
  return midiBuffer[fromBuffer];
}

unsigned char freeVoice(unsigned char note){

  unsigned char use=255;
  for(int i=ZERO;i<NUMBER_OF_VOICES;i++){
    if(voiceUse[i]==note){
      voiceUse[i]=255, use=i;
      break;
    }
  }
  return use;

}

unsigned char getFreeVoice(unsigned char note){

  unsigned char use=255;
  for(int i=ZERO;i<NUMBER_OF_VOICES;i++){
    if(voiceUse[i]==255) {
      voiceUse[i]=note, use=i;
      break; 
    }
  }

  if(use<NUMBER_OF_VOICES) return use;


}


void putNoteIn(unsigned char note){

  // check if the note is already in the buffer if yes put it to the first position
  if(notesInBuffer<BUFFER_SIZE){
    if(notesInBuffer>ZERO){ 
      shiftBufferRight();
    }

    midiBuffer[ZERO]=note; // put the last note to the first place 
    notesInBuffer++;
    if(notesInBuffer>POLYPHONY-1) freeVoice(midiBuffer[POLYPHONY]);
    thereIsNoteToPlay=true;
    fromBuffer=ZERO;
  }
}



unsigned char putNoteOut(unsigned char note){

  if(notesInBuffer>ZERO){ 
    unsigned char takenOut;
    boolean takeOut=ZERO;

    for(int i=ZERO;i<notesInBuffer;i++){
      if(midiBuffer[i]==note) takeOut=true, takenOut=i;
    } 

    if(takeOut){
      shiftBufferLeft(takenOut);
      notesInBuffer--;
      for(int i=notesInBuffer;i<BUFFER_SIZE;i++) midiBuffer[i]=ZERO;
      if(takenOut<POLYPHONY && notesInBuffer>=POLYPHONY) {
        thereIsNoteToPlay=true, fromBuffer=POLYPHONY-1;
      }
      return freeVoice(note);

    }

  }


  // put the last note to the first place 

}



void HandleNoteOn(byte channel, byte note, byte velocity) { 
  if(channel==inputChannel){
    if (velocity == ZERO) {
      HandleNoteOff(channel,note,velocity);

    }
    else{
      if(note<6){
        playSound(note,velocity);
        midiSound=note%6;
      }

      else{
        putNoteIn(note);
        if(isThereNoteToPlay()) {
          unsigned char _note=noteToPlay();
          unsigned char voice=getFreeVoice(_note);
          //  ADSR[voice].noteOff();
          playSound(sound,voice,_note,velocity);
        }

        //  midiSound=note%6;


      }
    }
  }
}

void HandleNoteOff(byte channel, byte note, byte velocity){


  if(channel==inputChannel){

    if(note<6){
      unsigned char voice=note%3;
      unsigned char _sound=note%6;

      if(currentSound[voice]==_sound) ADSR[voice].noteOff();
    }
    else{
      unsigned char outVoice=putNoteOut(note);

      if(isThereNoteToPlay()) {
        unsigned char _note=noteToPlay();
        unsigned char voice=getFreeVoice(_note);
        ADSR[outVoice].noteOff();
        // =getFreeVoice(_note);
        playSound(sound,voice,_note,velocity);
      }
      else{
        if(outVoice<NUMBER_OF_VOICES) ADSR[outVoice].noteOff();
      }

      //  if(notesInBuffer==0) for(int i=0;i<NUMBER_OF_VOICES;i++) ADSR[i].noteOff(),voiceUse[i]=255; //pojistka

    }

  }
}

void HandleControlChange(byte channel, byte number, byte value){
  // implement knob movement
  if(channel==inputChannel){
    if((number-CONTROL_CHANGE_OFFSET )<NUMBER_OF_VARIABLES){
      setVar(midiSound,number-CONTROL_CHANGE_OFFSET,scale(value,CONTROL_CHANGE_BITS,variableDepth[number-CONTROL_CHANGE_OFFSET]));  
      hw.freezeAllKnobs();
      renderTweaking((number-CONTROL_CHANGE_OFFSET)/VARIABLES_PER_PAGE);
    }
  }

  if(number==SWITCH_BYTE){
    for(int i=ZERO;i<3;i++) hw.setSwitch(i,bitRead(value,i));
  }
  else if(number==PAGE_BYTE){
    page=value;
  }

}

void HandleProgramChange(byte channel, byte number  ){
  // implement preset change
}

void HandlePitchBend(byte channel, int bend){

}

void HandleSongSelect(byte songnumber){

}

void HandleClock(){

}
void HandleStart(){

}
void HandleContinue(){

}
void HandleStop(){

}

void initMidi(unsigned char _channel){

  MIDI.begin(ZERO);    
  inputChannel=_channel;
  indicateMidiChannel(_channel);

  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandleNoteOff(HandleNoteOff);

  MIDI.setHandlePitchBend(HandlePitchBend);

  MIDI.setHandleControlChange(HandleControlChange);
  MIDI.setHandleProgramChange(HandleProgramChange);
  MIDI.setHandleSystemExclusive(HandleSystemExclusive);
  MIDI.setHandleSongSelect(HandleSongSelect);

  MIDI.setHandleClock(HandleClock);
  MIDI.setHandleStart(HandleStart);
  MIDI.setHandleContinue(HandleContinue);
  MIDI.setHandleStop(HandleStop);

  //MIDI.turnThruOn(Full);
  // MIDI.turnThruOff();  
}

#define SECOND_LAYER_CHANNEL 10
void indicateMidiChannel(unsigned char _channel){

  boolean highChannel=false;
  if(_channel>=SECOND_LAYER_CHANNEL)   hw.setLed(LED_1,true), hw.setLed(LED_2,true), hw.setLed(LED_3,true), highChannel=true;
  for(int i=ZERO;i<NUMBER_OF_KNOBS;i++){
    hw.setLed(_channel-1-highChannel*9,false);
    hw.update();
    delay(ANIMATION_DELAY);
    hw.setLed(_channel-1-highChannel*9,true);
    hw.update();
    delay(ANIMATION_DELAY);
  }

}






























