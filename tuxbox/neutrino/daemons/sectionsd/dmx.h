/*
 * $Header: /cvs/tuxbox/apps/tuxbox/neutrino/daemons/sectionsd/dmx.h,v 1.9 2007/07/18 20:04:25 houdini Exp $
 *
 * DMX class (sectionsd) - d-box2 linux project
 *
 * (C) 2003 by thegoodguy <thegoodguy@berlios.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __sectionsd__dmx_h__
#define __sectionsd__dmx_h__

#include <pthread.h>
#include <vector>

class DMX
{
 private:
	struct section_id
	{
		unsigned char table_id;
		unsigned short table_extension_id;
		unsigned char section_number;
		unsigned char version_number;
		unsigned short transport_stream_id;
		unsigned short original_network_id;
		section_id *next;
	};

	int             fd;
	pthread_mutex_t pauselock;
	unsigned short  pID;
	unsigned short  dmxBufferSizeInKB;
	section_id *first_section;
	section_id first_skipped_section;
	int		current_service;

	inline bool isOpen(void) { return (fd != -1); }

	int immediate_start(void); /* mutex must be locked before and unlocked after this method */
	int immediate_stop(void);  /* mutex must be locked before and unlocked after this method */
	bool check_complete(const unsigned char table_id, const unsigned short extension_id, const unsigned char);

 public:
	struct s_filters
	{
		unsigned char filter;
		unsigned char mask;
	};

	std::vector<s_filters> filters;
	int                    filter_index;
	time_t                 lastChanged;

	int                    pauseCounter;
	int                    real_pauseCounter;
	pthread_cond_t         change_cond;
	pthread_mutex_t        start_stop_mutex;


	DMX(const unsigned short p, const unsigned short bufferSizeInKB);
	~DMX();

	int start(void);
	ssize_t read(char * const buf, const size_t buflength, const unsigned timeoutMInSeconds);
	void closefd(void);
	void addfilter(const unsigned char filter, const unsigned char mask);
	int stop(void);

	int pause(void); // increments pause_counter
	int unpause(void); // decrements pause_counter
	
	int real_pause(void);
	int real_unpause(void);

	int request_pause(void);
	int request_unpause(void);

	int change(const int new_filter_index); // locks while changing

	void lock(void);
	void unlock(void);

	char * getSection(const unsigned timeoutInMSeconds, int &timeouts);
	// section with size < 3 + 5 are skipped !
	int setPid(const unsigned short new_pid);
	int setCurrentService(int new_current_service);
};

#endif /* __sectionsd__dmx_h__ */

