#include "eavswitch.h"
#include <unistd.h>
#include <fcntl.h>
#include <dbox/avs_core.h>
#include <sys/ioctl.h>

#include "config.h"

#include <core/system/econfig.h>
#include <core/dvb/edvb.h>

/* sucks */

#define SAAIOGREG               1 /* read registers                             */
#define SAAIOSINP               2 /* input control                              */
#define SAAIOSOUT               3 /* output control                     */
#define SAAIOSENC               4 /* set encoder (pal/ntsc)             */
#define SAAIOSMODE              5 /* set mode (rgb/fbas/svideo) */

#define SAA_MODE_RGB    0
#define SAA_MODE_FBAS   1
#define SAA_MODE_SVIDEO 2

#define SAA_NTSC                0
#define SAA_PAL                 1

#define SAA_INP_MP1             1
#define SAA_INP_MP2             2
#define SAA_INP_CSYNC   4
#define SAA_INP_DEMOFF  6
#define SAA_INP_SYMP    8
#define SAA_INP_CBENB   128

eAVSwitch *eAVSwitch::instance=0;

eAVSwitch::eAVSwitch()
{
	active=0;
	if (!instance)
		instance=this;
	fd=open("/dev/dbox/avs0", O_RDWR);
	saafd=open("/dev/dbox/saa0", O_RDWR);
	reloadSettings();

		// initial volume settings
	if (eConfig::getInstance()->getKey("/elitedvb/audio/volume", volume))
		volume=10;

	if (eConfig::getInstance()->getKey("/elitedvb/audio/VCRvolume", VCRVolume))
		VCRVolume=10;

	if (eConfig::getInstance()->getKey("/elitedvb/audio/mute", mute))
		mute=0;

	mute = !mute;
	toggleMute();
	muteAviaAudio(0);
	setInput(0);
}

eAVSwitch *eAVSwitch::getInstance()
{
	return instance;
}

eAVSwitch::~eAVSwitch()
{
	eConfig::getInstance()->setKey("/elitedvb/audio/volume", volume);
	eConfig::getInstance()->setKey("/elitedvb/audio/VCRvolume", VCRVolume);
	eConfig::getInstance()->setKey("/elitedvb/audio/mute", mute);

	if (instance==this)
		instance=0;

	if (fd>=0)
		close(fd);
	if (saafd>=0)
		close(saafd);
}


void eAVSwitch::reloadSettings()
{
	unsigned int colorformat;
	eConfig::getInstance()->getKey("/elitedvb/video/colorformat", colorformat);
	setColorFormat((eAVColorFormat)colorformat);
}

int eAVSwitch::setVolume(int vol)
{
	vol=63-vol/(65536/64);
	if (vol<0)
		vol=0;
	if (vol>63)
		vol=63;

	return ioctl(fd, AVSIOSVOL, &vol);
}

void eAVSwitch::changeVolume(int abs, int vol)
{
	switch (abs)
	{
		case 0:
			volume+=vol;
			if (mute)
				toggleMute();
		break;
		case 1:
			volume=vol;
			if (mute)
				toggleMute();
		break;
	}

	if (volume<0)
		volume=0;

	if (volume>63)
		volume=63;

	setVolume( (63-volume) * 65536/64 );

	sendVolumeChanged();
}

void eAVSwitch::changeVCRVolume(int abs, int vol)
{
	switch (abs)
	{
		case 0:
			VCRVolume+=vol;
			if (mute)
				toggleMute();
		break;
		case 1:
			VCRVolume=vol;
			if (mute)
				toggleMute();
		break;
	}

	if (VCRVolume<0)
		VCRVolume=0;

	if (VCRVolume>63)
		VCRVolume=63;

	setVolume( (63-VCRVolume) * 65536/64 );

	/*emit*/ volumeChanged(VCRVolume);
}


void eAVSwitch::sendVolumeChanged()
{
	/*emit*/ volumeChanged(mute?63:volume);
}

void eAVSwitch::toggleMute()
{
	mute = !mute;

	if (mute)
	{
		setVolume(63);
		sendVolumeChanged();
	}
	else
		changeVolume(1,volume);
}

void eAVSwitch::muteAviaAudio(bool m)
{
	int a;

	if(m)
		a=AVS_MUTE;
	else
		a=AVS_UNMUTE;

	if (ioctl(fd, AVSIOSMUTE, &a) < 0)
	{
		perror("AVSIOSMUTE:");
		return;
	}
}

int eAVSwitch::setTVPin8(int vol)
{
/* results from philips:	fnc=0 -> 0V
				fnc=1 -> 0V
				fnc=2 -> 6V
				fnc=3 -> 12V
*/
	int fnc;
	switch (vol)
	{
	case 0:
		fnc=(Type==PHILIPS?1:0);
		break;
	case 6:
		fnc=(Type==PHILIPS?2:1);
		break;
	case 12:
		fnc=(Type==PHILIPS?3:2);
		break;
	}
	return ioctl(fd, AVSIOSFNC, &fnc);
}

int eAVSwitch::setColorFormat(eAVColorFormat c)
{
	colorformat=c;
	int arg=0;
	switch (c)
	{
	case cfNull:
		return -1;
	case cfCVBS:
		arg=SAA_MODE_FBAS;
		break;
	case cfRGB:
		arg=SAA_MODE_RGB;
		break;
	case cfYC:
		arg=SAA_MODE_SVIDEO;
		break;
	}
	int fblk = (c == cfRGB)?1:0;
	ioctl(saafd, SAAIOSMODE, &arg);
	ioctl(fd, AVSIOSFBLK, &fblk);
	return 0;
}

int eAVSwitch::setInput(int v)
{	
	eDebug("[eAVSwitch] setInput %d, fd=%d", v, fd);
	switch (v)
	{
	case 0:	//	Switch to DVB
		ioctl(fd, AVSIOSVSW1, dvb);
		ioctl(fd, AVSIOSASW1, dvb+1);
		ioctl(fd, AVSIOSVSW2, dvb+2);
		ioctl(fd, AVSIOSASW2, dvb+3);
		ioctl(fd, AVSIOSVSW3, dvb+4);
		ioctl(fd, AVSIOSASW3, dvb+5);
		changeVolume(1, volume);  // set Volume to TV Volume
		reloadSettings();
		break;
	case 1:   // Switch to VCR
		v = (Type == SAGEM)? 0 : 2;
		ioctl(fd, AVSIOSFBLK, &v);
		ioctl(fd, AVSIOSVSW1, scart);
		ioctl(fd, AVSIOSASW1, scart+1);
		ioctl(fd, AVSIOSVSW2, scart+2);
		ioctl(fd, AVSIOSASW2, scart+3);
		ioctl(fd, AVSIOSVSW3, scart+4);
		ioctl(fd, AVSIOSASW3, scart+5);
		changeVCRVolume(1, VCRVolume);
		break;
	}
	return 0;
}

int eAVSwitch::setAspectRatio(eAVAspectRatio as)
{
	aspect=as;
	return setTVPin8(active?((aspect==r169)?6:12):0);
}

int eAVSwitch::isVCRActive()
{
	return 0;
}

int eAVSwitch::setActive(int a)
{
	active=a;
	return setTVPin8(active?((aspect==r169)?6:12):0);
}

bool eAVSwitch::loadScartConfig()
{
	FILE* fd = fopen(CONFIGDIR"/scart.conf", "r");
	if(fd)
	{
		eDebug("[eAVSwitch] loading scart-config (scart.conf)");

		char buf[1000];
		fgets(buf,sizeof(buf),fd);

		eString readline_scart = "_scart: %d %d %d %d %d %d\n";
		eString readline_dvb = "_dvb: %d %d %d %d %d %d\n";

		switch (Type)
		{
			case NOKIA:
				readline_scart.insert(0, "nokia");
				readline_dvb.insert(0, "nokia");
			break;
			case PHILIPS:
				readline_scart.insert(0, "philips");
				readline_dvb.insert(0, "philips");
			break;
			case SAGEM:
				readline_scart.insert(0, "sagem");
				readline_dvb.insert(0, "sagem");
			break;
		}

		int i=0;
		while ( fgets(buf,sizeof(buf),fd) != NULL && (i < 3) )
		{
			if ( !(1&i) && sscanf( buf, readline_scart.c_str(), &scart[0], &scart[1], &scart[2], &scart[3], &scart[4], &scart[5] ) == 6)
				i |= 1;
			else if ( !(2&i) && sscanf( buf, readline_dvb.c_str(), &dvb[0], &dvb[1], &dvb[2], &dvb[3], &dvb[4], &dvb[5]) == 6 )
				i |= 2;
		}

		if ( i != 3 )
			eDebug( "[eAVSwitch] failed to parse scart-config (scart.conf), using default-values" );

		eDebug("[eAVSwitch] readed scart conf : %i %i %i %i %i %i", scart[0], scart[1], scart[2], scart[3], scart[4], scart[5] );
		eDebug("[eAVSwitch] readed dvb conf : %i %i %i %i %i %i", dvb[0], dvb[1], dvb[2], dvb[3], dvb[4], dvb[5] );
	
		fclose(fd);
	}
	else
	{
		eDebug("[eAVSwitch] failed to load scart-config (scart.conf), using default-values");
	}
	return 0;
}
