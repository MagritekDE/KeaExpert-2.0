#include "stdafx.h"
#include "sounds.h"
#include "globals.h"
#include "process.h"
#include <mmsystem.h>
#include "scanstrings.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"

int SaveWaveFile(Interface* itfc, char args[]);

/*******************************************************************************
  Play the sound stored in vSound at specified sampling rate using 16 bit audio.
*******************************************************************************/

int ProduceSound(Interface* itfc, char args[])
{
   short nrArgs;
   float sampingRate = 8192; // Default sampling rate
   Variable vSound;
   short *sdata;
   long samples;
	WAVEFORMATEX wfme;
	MMRESULT mmres;
	HWAVEOUT hwout;
	WAVEHDR whdr;
   CText scaling = "scale";
   short nrChannels;

// Read in the sound variable and the sampling rate
   if((nrArgs = ArgScan(itfc,args,1,"sound, [sps, [fixed/scale]]","eee","vft",&vSound,&sampingRate,&scaling)) < 0)
      return(nrArgs); 

// Get the mono data and convert to short format
   if(VarType(&vSound) == MATRIX2D && VarHeight(&vSound) == 1)
   {
      samples = VarWidth(&vSound);
      float *fdata = VarRealMatrix(&vSound)[0];
      nrChannels = 1;

      // Convert input data to signed 16 bit sdata
      sdata = new short[samples];

      if(scaling == "scale") // Scale the data to be 16 bit 
      {
         float maxV = -1e38;
         float minV = 1e38;
         for(int i = 0; i < samples; i++)
         {
            if(fdata[i] > maxV) maxV = fdata[i];
            if(fdata[i] < minV) minV = fdata[i];
         }

         for(int i = 0; i < samples; i++)
         {
            sdata[i] = (short)(32767*(fdata[i]-minV)/(maxV-minV));   
         }
      }
      else if(scaling == "fixed") // Just limit the data to +-32767
      {
         float data;
         for(int i = 0; i < samples; i++)
         {
            data = fdata[i];
            if(data > 32767.0) data = 32767.0;
            if(data < -32768.0) data = -32768.0;
            sdata[i] = (short)data;   
         }
      }
      else
      {
			delete[] sdata;
         ErrorMessage("invalid scaling value");
         return(ERR);
      }
   }
// Get the stereo data and convert to short format
   else if(VarType(&vSound) == MATRIX2D && VarHeight(&vSound) == 2)
   {
      samples = VarWidth(&vSound);
      float **fdata = VarRealMatrix(&vSound);

      // Convert input data to signed 16 bit sdata
      sdata = new short[samples*2];
      nrChannels = 2;


      if(scaling == "scale") // Scale the data to be 16 bit 
      {
         float maxV = -1e38;
         float minV = 1e38;
         for(int i = 0; i < samples; i++)
         {
            if(fdata[0][i] > maxV) maxV = fdata[0][i];
            if(fdata[1][i] > maxV) maxV = fdata[1][i];
            if(fdata[0][i] < minV) minV = fdata[0][i];
            if(fdata[1][i] < minV) minV = fdata[1][i];
         }

         for(int j = 0, i = 0; i < samples; i++)
         {
            sdata[j++] = (short)(32767*(fdata[0][i]-minV)/(maxV-minV));   
            sdata[j++] = (short)(32767*(fdata[1][i]-minV)/(maxV-minV));   
         }
      }
      else if(scaling == "fixed") // Just limit the data to +-32767
      {
         float dataR,dataL;
         for(int i = 0, j = 0; i < samples; i++)
         {
            dataR = fdata[0][i];
            dataL = fdata[1][i];
            if(dataR > 32767.0) dataR = 32767.0;
            if(dataL > 32767.0) dataL = 32767.0;
            if(dataR < -32768.0) dataR = -32768.0;
            if(dataL < -32768.0) dataL = -32768.0;
            sdata[j++] = (short)dataR;   
            sdata[j++] = (short)dataL;   
         }
      }
      else
      {
			delete[] sdata;
         ErrorMessage("invalid scaling value");
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("invalid sound data");
      return(ERR);
   }


// Set up the wave format
	wfme.wFormatTag = WAVE_FORMAT_PCM;
	wfme.nChannels = nrChannels;
	wfme.nSamplesPerSec = sampingRate;
	wfme.wBitsPerSample = 16;
	wfme.nBlockAlign = wfme.nChannels*wfme.wBitsPerSample/8;
	wfme.nAvgBytesPerSec = sampingRate*wfme.nBlockAlign;
	wfme.cbSize = 0;
   hwout = NULL;

// Open the wave object
	mmres = waveOutOpen(&hwout, WAVE_MAPPER, &wfme, 0L, 0L, CALLBACK_NULL);
	if (mmres != MMSYSERR_NOERROR)
	{
      ErrorMessage("Can't open wave output");
      delete [] sdata;
		return FALSE;
	}

// Prepare the wave header
   whdr.dwFlags = WHDR_BEGINLOOP;
   whdr.lpData = (LPSTR)sdata;
   whdr.dwBufferLength = samples;
   whdr.dwLoops = 1;
	mmres = waveOutPrepareHeader(hwout, &whdr, sizeof(WAVEHDR));
	if (mmres != MMSYSERR_NOERROR)
	{
      ErrorMessage("Can't prepare wave header");
		waveOutClose(hwout);
		delete [] sdata;
		return(ERR);
	}
	whdr.dwLoops = 0;
// Play the sound - allowing user to abort if desired
	mmres = waveOutWrite(hwout, &whdr, sizeof(WAVEHDR));
	if (mmres == MMSYSERR_NOERROR)
	{
		while (!(whdr.dwFlags & WHDR_DONE))
      {
         if(ProcessBackgroundEvents() != OK)
         {
            waveOutReset(hwout);
	         waveOutUnprepareHeader(hwout, &whdr, sizeof(WAVEHDR));
	         waveOutClose(hwout);
            delete [] sdata;
	         return(ABORT);
         }
      }
	}
// Close the wave object
	waveOutUnprepareHeader(hwout, &whdr, sizeof(WAVEHDR));
	waveOutClose(hwout);
   delete [] sdata;
	return(OK);
}


struct WAVHeader
{
    char ChunkID[4];
    UINT32 ChunkSize;
    char RIFFType[4];
};

struct FormatHeader
{
    char ChunkID[4];
    UINT32 ChunkSize;
    UINT16 CompressionCode;
    UINT16 Channels;
    UINT32 SampleRate;
    UINT32 AvgBytesPerSec;
    UINT16 BlockAlign;
    UINT16 SigBitsPerSamp;
};

struct DataHeader
{
    char ChunkID[4];
    UINT32 ChunkSize;
};


/*******************************************************************************
  Save a vector as a wav file.
*******************************************************************************/


int SaveWaveFile(Interface* itfc, char args[])
{
   char * output = NULL;
   int i = 0;
   CText fileName;
   Variable vSound;
   short nrArgs;
   long sampingRate = 8192; // Default sampling rate

   // Get filename and vector
   if((nrArgs = ArgScan(itfc,args,2,"filename, vector, sampling_rate","eee","tvl",&fileName,&vSound,&sampingRate)) < 0)
      return(nrArgs); 

   if(vSound.GetType() != MATRIX2D)
   {
      ErrorMessage("Sound vector is not a matrix");
      return(ERR);
   }

   if(vSound.GetDimY() != 1)
   {
      ErrorMessage("Sound vector is not row vector");
      return(ERR);
   }

   int length = vSound.GetDimX();
   float *vec = vSound.GetMatrix2D()[0];

   //Allocate memory for wav file
   size_t size = sizeof(struct WAVHeader) + sizeof(struct FormatHeader) + sizeof(struct DataHeader) + length*sizeof(short);
   void * buffer = malloc(size);

   //Fill buffer with headers
   struct WAVHeader *WAV = (struct WAVHeader *)buffer;
   struct FormatHeader *Format = (struct FormatHeader *)(WAV + 1);
   struct DataHeader *Data = (struct DataHeader *)(Format + 1);

   strcpy(WAV->ChunkID, "RIFF");
   WAV->ChunkSize = (UINT32)size - 8;
   strcpy(WAV->RIFFType, "WAVE");

   strcpy(Format->ChunkID, "fmt ");
   Format->ChunkSize = 16;
   Format->CompressionCode = 1;
   Format->Channels = 1;
   Format->SampleRate = (UINT32)sampingRate;
   Format->SigBitsPerSamp = 16;
   Format->BlockAlign = 2;
   Format->AvgBytesPerSec = Format->BlockAlign * sampingRate;

   strcpy(Data->ChunkID, "data");
   Data->ChunkSize = length;

   // Add Sound
   short * sound = (short *)(Data + 1);
   for (int x = 0; x < length; x++)
   {
       sound[x] = (short)(vec[x]+0.5);
   }

   // Write buffer to file
   FILE * fp = fopen(fileName.Str(), "wb");
   if(fp)
   {
      fwrite(buffer, size, 1, fp);
      fclose(fp);
   }
   else
   {
      ErrorMessage("Can't write to file '%s'",fileName.Str());
      return(ERR);
   }

   return(OK);
}