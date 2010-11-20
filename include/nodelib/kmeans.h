
/* Copyright (c) 1996 by G. W. Flake.
 *
 * NAME
 *   kmeans.h - perform k-means clustering on a DATASET
 * SYNOPSIS
 *   The routines in this module will compute clusters of a
 *   DATASET with the k-means clustering algorithm.  The resulting
 *   clusters are returned in another DATASET.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 * CREDITS
 *   The original source code for this package came from Chris Darken
 *   (\bf{darken@scr.siemens.com}).  Thanks Chris.
 * SEE ALSO
 *   \bf{series}(3), and \bf{dsmethod}(3).
 */

#ifndef __KMEANS_H__
#define __KMEANS_H__

#include "nodelib/dataset.h"
#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This routine will take all of the points in \em{set} and attempt to
   cluster them into \em{nclus} points via the k-means clustering
   algorithm.  The routine's termination is determinined by the last
   two arguments, with \em{minfrac} being a tolerance on the
   fractional decrease in the distortion for one iteration, and
   \em{maxiters} is the maximum number of iterations.  If \em{minfrac}
   is less than zero, then the routine will work for \em{maxiters}
   iterations, but if \em{maxiters} is zero, then the routines will
   only consider \em{minfrac.} It is an error to set \em{minfrac} less
   than zero with \em{maxiters} set to zero.  If \em{clusinit} is
   zero, the clusters are initialized to random exemplars.  If
   \em{clusinit} is one, the clusters are initialized to the means of
   a random partitioning of the entire data set.  If \em{clusinit} is
   two, the clusters are initialized to the means of a sequential
   partitioning of the entire data set.  The resulting clusters can be
   found in the DATASET returned.  If any error occurs, NULL is
   returned. */

DATASET *kmeans(DATASET *set, unsigned nclus, double minfrac, /*\*/
                unsigned maxiters, int clusinit);


/* Given \em{set} (which has n x-dimensional data points), perform
   online kmeans clustering and return \em{nclus} clusters in the
   returned DATASET.  The algorithm operates on \em{maxiters} shuffled
   samples, reshuffling the data set as necessary.  If \em{clusinit}
   is zero, the clusters are initialized to random exemplars.  This is
   the usual choice for online kmeans.  If \em{clusinit} is one, the
   clusters are initialized to the means of a random partitioning of
   the entire data set.  If \em{clusinit} is two, the clusters are
   initialized to the means of a sequential partitioning of the entire
   data set. If any error occurs, NULL is returned. */

DATASET *kmeans_online(DATASET *set, unsigned nclus, unsigned maxiters, /*\*/
                       int clusinit);

/* Valid values for 'clusinit' */

#define KMEANS_RANDOM_EXEMPLARS 0
#define KMEANS_RANDOM_PARTITION 1
#define KMEANS_SEQ_PARTITION    2

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KMEANS_H__ */
