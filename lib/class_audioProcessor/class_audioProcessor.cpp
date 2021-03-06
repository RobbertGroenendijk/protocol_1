#include "Arduino.h"
#include "class_audioProcessor.h"

AudioProcessor::AudioProcessor() {

}
void AudioProcessor::setup(AudioControlSGTL5000* _sgtl5000,AudioSynthWaveformSine* _sine, AudioMixer4* _mixer1, AudioMixer4* _mixer2,AudioAnalyzeFFT1024* _fft1024) {

  sgtl5000 = _sgtl5000;
  sine1 = _sine;
  mixer1 = _mixer1;
  mixer2 = _mixer2;
  fft1024_1 = _fft1024;

  AudioMemory(10);
  sgtl5000->enable();
  sgtl5000->volume(0.5);
  sgtl5000->inputSelect(AUDIO_INPUT_MIC);
  sgtl5000->micGain(90);
  sine1->amplitude(0.5);

  int mixerChannelCount = 4;
  for (int i = 0; i < mixerChannelCount; i ++) {
    mixer1->gain(i,0.75);
    mixer2->gain(i,0.75);
  }
  arrayIndex = 0;
  previousMillis = millis();
}
void AudioProcessor::reset() {
  flushFft();
  int mixerChannelCount = 4;
  for (int i = 0; i < mixerChannelCount; i ++) {
    mixer1->gain(i,0.75);
    mixer2->gain(i,0.75);
  }
  previousMillis = millis();

  Serial.println("AUDIOPROCESSOR reset");
}
void AudioProcessor::printFFT() {
  if (fft1024_1->available()) {
    // each time new FFT data is available
    // print it all to the Arduino Serial Monitor
    //Serial.print("FFT: ");
    for (int i=0; i<40; i++) {
      float n = fft1024_1->read(i);
      if (n >= 0.01) {
        //Serial.print(n);
        //Serial.print(" ");
      } else {
        //Serial.print("  -  "); // don't print "0.00"
      }
    }
    Serial.println();
  }
}
void AudioProcessor::testTone(char _c) {
  if (_c == 'r') {
    sine1->frequency(1000);
    toggleMixer(2,0.75);
    delay(200);
    sine1->frequency(2000);
    toggleMixer(2,0.75);
    delay(200);
    sine1->frequency(3000);
    toggleMixer(2,0.75);
    delay(200);
    stopTone();
  } else if (_c == 's') {
    sine1->frequency(5000);
    toggleMixer(2,0.75);
    delay(200);
    sine1->frequency(4000);
    toggleMixer(2,0.75);
    delay(200);
    sine1->frequency(3000);
    toggleMixer(2,0.75);
    delay(200);
    stopTone();
  }
};
void AudioProcessor::sendTone(int _frequency) {
  sine1->frequency(_frequency);
  toggleMixer(2,0.75);
};
void AudioProcessor::stopTone() {
  mixer1->gain(0,0.0);
  mixer1->gain(1,0.0);
  mixer1->gain(2,0.0);
  mixer1->gain(3,0.0);

  mixer2->gain(0,0.0);
  mixer2->gain(1,0.0);
  mixer2->gain(2,0.0);
  mixer2->gain(3,0.0);
};
int AudioProcessor::getHighestFFTBand(int _beginBand, int _endBand) {
  int highestBand = 0;
  float highestAmplitude = 0;

        for (int i = _beginBand; i < _endBand; i++) {
          float fftNumber = fft1024_1->read(i);
          //Serial.println(fftNumber);

          if (fftNumber > highestAmplitude) {
            highestAmplitude = fftNumber;
            highestBand = i;
          }
        }
        Serial.print("Current highest FFT band: ");
        Serial.println(highestBand);
        return highestBand;
}
void AudioProcessor::saveSignal() {
  currentMillis = millis();
  unsigned long elapsedTime = currentMillis - previousMillis;
  // Listen for 1 second every 5 milliseconds
  if (elapsedTime > 5) {

    toggleMixer(1,0.75);
    fftBandArray[arrayIndex] = getHighestFFTBand(0,255);

    if (arrayIndex < 200) {
      arrayIndex += 1;
      previousMillis = millis();
    } else {
      arrayIndex = 0;
      previousMillis = millis();
    }
  }
}
int AudioProcessor::getSignal() {
  //Serial.println(fftBandArray[123]);
  unsigned int occurNumber = 0;
  unsigned int index;
  unsigned int currentOccurNumber = 0;

  for (int i = 0; i < 200; i++) {
    for (int j = 0; j < 200; j++) {
      if (fftBandArray[i] > 0) {
        if (fftBandArray[i] == fftBandArray[j]) {

          currentOccurNumber += 1;
        }
      }
    }
    if (currentOccurNumber > occurNumber) {
      occurNumber = currentOccurNumber;
      index = i;
    }
  }
  //Serial.print("most occurring fft number:");
  //Serial.println(fftBandArray[index]);

  return fftBandArray[index];

}
void AudioProcessor::toggleMixer(int _mixerNumber, float _volume) {
  if (_mixerNumber == 1) {
    mixer1->gain(0,_volume);
    mixer1->gain(1,_volume);
  } else {
    mixer2->gain(0,_volume);
    mixer2->gain(1,_volume);
  }
}
void AudioProcessor::flushFft() {
  for (int i = 0; i < 200; i++){
    fftBandArray[i] = 0;
  }
}
