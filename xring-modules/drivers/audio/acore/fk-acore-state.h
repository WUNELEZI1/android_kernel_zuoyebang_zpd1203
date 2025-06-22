/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _FK_ACORE_STATE_
#define _FK_ACORE_STATE_

#include "fk-acore.h"

/****************************** enum define begin *************************/


struct acore_stream_ops {
	int (*startup)(struct acore_stream *ac_strm);
	int (*shutdown)(struct acore_stream *ac_strm);
	int (*start)(struct acore_stream *ac_strm);
	int (*pause)(struct acore_stream *ac_strm);
	int (*flush)(struct acore_stream *ac_strm);
	int (*resume)(struct acore_stream *ac_strm);
	int (*stop)(struct acore_stream *ac_strm);
	int (*drain)(struct acore_stream *ac_strm);
	int (*next_track)(struct acore_stream *ac_strm);
};

struct acore_stream_ops *fk_stream_ops_get(struct acore_stream *ac_strm);


/****************************** enum define end *************************/


#endif /* _FK_ACORE_STATE_ */
