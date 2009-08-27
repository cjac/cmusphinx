/**
 \file lm_max.h
 \brief Maximum probabilities computing
	 
 This is the header file for compute and use maximums back-off
 weight for all trigrams ended by any words w2,w3 and maximum
 probabilities for all quadrigrams ended by any words w2,w3,w4.
 This file uses Language Model in 32-bits mode.
*/
/*
 HISTORY
 2008/07/08  N. Coetmeur, supervised by Y. Esteve
 Creating
*/


#ifndef _LM_MAX_H_
#define _LM_MAX_H_

#include "s3types.h"
#include "lm.h"


/**
 \struct tg_max_prob_t
 \brief Trigram with a maximum probability
*/
typedef struct tg_max_prob_t
{
	s3lmwid32_t	wid;		/**< Word ID */
	lmlog_t		max_prob;	/**< Maximum back-off weight */
} tg_max_prob_t;

/**
 \struct bg_max_bowt_t
 \brief Bigram with a maximum back-off weight
*/
typedef struct bg_max_bowt_t
{
	s3lmwid32_t	wid;		/**< Word ID */
	lmlog_t		max_bowt;	/**< Maximum back-off weight */
	uint32		firsttg;	/**< First trigram corresponding */
} bg_max_bowt_t;

/**
 \struct tg_max_prob_node_t
 \brief Trigram node in a linked list
*/
typedef struct tg_max_prob_node_t
{
	tg_max_prob_t				tg;		/**< Trigram */

	struct tg_max_prob_node_t *	next;	/**< Next entry in linked list */
} tg_max_prob_node_t;

/**
 \struct bg_max_bowt_node_t
 \brief Bigram node in a linked list
*/
typedef struct bg_max_bowt_node_t
{
	bg_max_bowt_t					bg;			/**< Bigram */

	struct bg_max_bowt_node_t	*	next;		/**< Next entry in linked list */

	tg_max_prob_node_t			*	firsttgn;	/**< First trigram node */
	tg_max_prob_node_t			*	cur_tgn;	/**< Current trigram node */
	uint32							n_tgn;		/**< Number of trigrams
																corresponding */
} bg_max_bowt_node_t;

/**
 \struct ug_max_node_t
 \brief Unigram node pointed to a bigrams linked list
*/
typedef struct ug_max_node_t
{
	bg_max_bowt_node_t *	firstbgn;	/**< Start of bigrams list */
	bg_max_bowt_node_t *	cur_bgn;	/**< Current bigram node */
	uint32					n_bgn;		/**< Number of bigrams corresponding */
} ug_max_node_t;

/**
 \struct lm_max_t
 \brief	LM Maximums computing model

 Specific LM model for compute maximums probabilities and back-off weights.
*/
typedef struct lm_max_t
{
	/** \name Numbers of N-grams */
	/*\{*/
	uint32 n_ug; /**< Unigrams */
	uint32 n_bg; /**< Bigrams */
	uint32 n_tg; /**< Tigrams */
	/*\}*/

	ug_max_node_t * ug_list; /**< Table of unigram nodes pointed to
														 bigrams linked lists */

	/** \name Model with tables of N-grams */
	/*\{*/
	uint32			* ug_table; /**< Table of first bigram indexes for each
					   unigram word (the table index is equal to the word ID) */
	bg_max_bowt_t	* bg_table; /**< Bigrams table */
	tg_max_prob_t	* tg_table; /**< Trigrams table */
	/*\}*/
	
	uint8 use_linked_lists; /**< Indicates if model uses linked lists or
																simple tables */
} lm_max_t;


/**
  \brief Read all quadrigrams for compute maximums probabilities
	  \return Error value
*/
int
lm_max_compute	(  lm_t		* lm,		/**< In: Language model */
				   lm_max_t	* lm_max	/**< In/Out: Set of maximums
										   probabilities and back-off weights */
				   );

/**
  \brief Read a LM Max model from a MAX file

  Read maximums model in tables mode

	  \return Allocated model
*/
lm_max_t *
lm_max_read_dump	(const char *	filename,		/**< In: Dump file name */
					 int32			applyweight,	/**< In: whether lw, wip
										should be applied to the model or not */
					 float64		lw,				/**< In: language weight */
					 float64		wip				/**< In: word insertion
                                                                      penalty */
					 );

/**
  \brief Locate a specific bigram within a bigram table
    \return Index of bigram found (nsearch if not found)
*/
uint32
find_bg	(bg_max_bowt_t *	table,      /**< In: The bigram table */
         int32				nsearch,	/**< In: Maximum number of bigrams */ 
         s3lmwid32_t		w			/**< In: The last word of bigram */
         );

/**
  \brief Locate a specific trigram within a trigram table
    \return Index of bigram found (nsearch if not found)
*/
uint32
find_tg	(tg_max_prob_t *	table,      /**< In: The trigram table */
         int32				nsearch,	/**< In: Maximum number of trigrams */ 
         s3lmwid32_t		w			/**< In: The last word of trigram */
         );

/**
  \brief Return the maximum probability of all quadrigrams ended by w2,w3,w4
	  \return Error code (0 if trigram found)
*/
int
lm_max_get_prob	(lm_max_t	*	lm_max,	/**< In: Maximums model */
				 lmlog_t	*	prob,	/**< Out: Probability found */
				 s3lmwid32_t	w2,		/**< In: Second word ID of quadrigram */
				 s3lmwid32_t	w3,		/**< In: Third word ID of quadrigram */
				 s3lmwid32_t	w4		/**< In: Fourth word ID of quadrigram */
					);

/**
  \brief Return the maximum back-off weight of all trigrams ended by w2,w3
	  \return Error code (0 if trigram found)
*/
int
lm_max_get_bowt	(lm_max_t	*	lm_max,	/**< In: Maximums model */
				 lmlog_t	*	bowt,	/**< Out: Back-off weight found */
				 s3lmwid32_t	w2,		/**< In: Second word ID of trigram */
				 s3lmwid32_t	w3		/**< In: Fourth word ID of trigram */
					);

/**
  \brief Write maximums list in a dump file
	\return Error code
*/
int
lm_max_write_dump	( const char	* dmp_file,	/**< In: Dump file */
					  lm_max_t		* lm_max	/**< In: Set of maximums
										   probabilities and back-off weights */
					  );

/**
 \brief Compute the maximum score for all quadrigrams ended by w2,w3,w4

 This functions compute the maximum score of
 existing and inexisting quadrigrams.
 
 \return The score
*/
int32
lm_best_4g_score	(lm_t			*	lm,		/**< In: Language model */
					 lm_max_t		*	lm_max,	/**< In: Model of maximum
					 probability for existing quadrigrams (ended by w2,w3,w4) */
					 s3lmwid32_t	*	lw,		/**< In: Table of three
																	 word IDs */
					 s3wid_t			wn		/**< In: Class ID */
					 );

/**
  \brief Allocate a LM Max model with linked lists
	  \return Allocated list
*/
lm_max_t *
lm_max_lists_new	(uint32 n_ug /**< In: Number of unigrams */
					 );

/**
  \brief Allocate a LM Max model with tables
	  \return Allocated model
*/
lm_max_t *
lm_max_tables_new	( uint32 n_ug,	/**< In: Number of unigrams */
					  uint32 n_bg,	/**< In: Number of bigrams */
					  uint32 n_tg	/**< In: Number of trigrams */
					  );

/**
  \brief Free the list of maximums probabilities and back-off weights
*/
void
lm_max_free	(lm_max_t *	lm_max	/**< In: Set of maximums
										   probabilities and back-off weights */
				);

#endif
