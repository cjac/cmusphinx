/*
 * audio_utils.h -- From Bob Brennan for Sun audio utilities.
 * 
 * HISTORY
 * 
 * 15-Jul-98	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		sys/audioio.h and sys/filio.h included following Alex 
 *              Rudnicky's remarks.
 * 
 * 02-Aug-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created from Bob Brennan's original.
 */


#include <sys/audioio.h>
#include <sys/filio.h>


int	audioOpen(int rate);
void	audioPause(void);
void	audioFlush(void);
void	audioStartRecord(void);
void	audioStopRecord(void);
void	audioClose(void);
int	audioSetRecordGain(int gain);
