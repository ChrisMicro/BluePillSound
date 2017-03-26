#ifdef __cplusplus
extern "C"
{
#endif

#ifndef WAVEGENERATOR_H_
#define WAVEGENERATOR_H_

	#include "dac.h"
	#include <stdint.h>

	typedef struct {
		// sin wave generator parameters
		uint32_t 	phase; 				// DDS phase accumulator
		uint32_t 	freq_coefficient; 	// DDS value by which the phase is incremented, calculated from frequency
		uint8_t 	envelope;
		const int8_t		*waveform;

		int32_t 	amplitude;

		// envelope parameters
		uint16_t 	attack_ms;
		uint16_t	decay_ms;
		uint16_t	release_ms;
		int16_t 	sustain_level;
		uint8_t 	attackdecay_flag; 	// True/FALSE
		uint8_t 	noteOn;
	} WaveGenerator;

	extern volatile uint32_t MsTimerForSound; // millisecond timer, increased in interrupt routine
	extern volatile WaveGenerator Wave[];
	extern const int8_t sintab[];
	extern const int8_t triangletab[];
	extern const int8_t squaretab[];

	__inline void waveGenerators(void);

	void ring_bell(uint16_t *freq,uint16_t *time);

	void updateEnvelope(void);
	void initWaveforms(void);
	void voiceOff(void);
	void organ(uint16_t frequency);

#endif /* WAVEGENERATOR_H_ */

#ifdef __cplusplus
}
#endif
