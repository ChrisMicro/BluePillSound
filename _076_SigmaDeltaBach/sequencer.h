/*
 * sequencer.h
 *
 *  Created on: 03.02.2013
 *      Author: chris
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef SEQUENCER_H_
#define SEQUENCER_H_


	#include "wavegenerator.h"

	uint16_t sequencer(void);
	void ms_loop(void);
	void noteOn(uint8_t channel, uint8_t note, uint8_t velocity);
	void noteOff(uint8_t channel, uint8_t note);
	uint32_t note2frequency(uint8_t);

#endif /* SEQUENCER_H_ */

#ifdef __cplusplus
}
#endif



