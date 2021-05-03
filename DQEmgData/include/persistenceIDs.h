/***************************************************************************

 PersstenceIDs.h    Declaration of Analysis object persistence id numbers.  These codes
               are embedded in a persistence stream and let a persistent object
               verify that a stream contains their class type before they
               attempt to restore themself.

 Author        Todd Veldhuizen

 Date          September 1993

 Revision History:

 ***************************************************************************/


#ifndef __EMGANALYSIS_IDs_H

#define __EMGANALYSIS_IDs_H

#define ID_FIRING					100
#define ID_HISTO					101
#define ID_JIGGLE					102
#define ID_MACROTMPL				103
#define ID_MICROTMPL				104
#define ID_EMGANALYSIS				105
#define ID_TRAINANLYSIS				106
#define ID_TEMPLATE					107
#define ID_SHIMMERGRAPH				108
//#define ID_MEDTEMP				109
#define ID_BASEGRAPH				110
//#define ID_TGGEOM					111
#define ID_ENSEMBLE_DATA			112
#define ID_JITTER					113
#define ID_JITTERPAIR				114	
#define ID_TURNDATA					115
#define ID_CMAP						116
//#define ID_TRAIN_ENSEMBLE_SUMMARY	117
#define ID_FIRING_DATA				118
#define ID_MUPTMPL					119
#define ID_TEMPLATEGRAPH			120
#define ID_MICROTEMPLATEGRAPH		121
#define ID_MACROTEMPLATEGRAPH		121

#define ID_SMUP						122
#define ID_MUNE						123
#define ID_STUDY					124

// EH: This 200 level series of IDs allows us to make changes to
// template blocks and use a new ID to indicate that a different
// reading/writing procedure is required. Must have the same two
// ending digits as its 100 level encoding. All occurrences of
// the 100 level ID can be replaced with the 200 level ID.
#define ID_CMAP2					216
#define ID_MUPTMPL2					219
#define ID_MICROTMPL3				304
#define ID_MUPTMPL3					419

#endif

/**
 ** $Log: persistenceIDs.h,v $
 ** Revision 1.1  2006/07/14 17:11:25  stashuk
 ** ...
 **
 **/

