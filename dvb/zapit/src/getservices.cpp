/*
 * $Id: getservices.cpp,v 1.67.2.8 2003/05/15 20:37:15 digi_casi Exp $
 *
 * (C) 2002, 2003 by Andreas Oberritter <obi@tuxbox.org>
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

#include <zapit/bouquets.h>
#include <zapit/channel.h>
#include <zapit/debug.h>
#include <zapit/frontend.h>
#include <zapit/getservices.h>
#include <zapit/settings.h>
#include <zapit/xmlinterface.h>

extern std::map <uint32_t, transponder> transponders;
extern tallchans allchans;

std::map <string, int32_t> satellitePositions; //satellite position as specified in satellites.xml
std::map <string, uint8_t> motorPositions; //stored satellitepositions in diseqc 1.2 motor

void ParseTransponders(xmlNodePtr node, const uint8_t DiSEqC, std::string satellite, int32_t satellitePosition)
{
	t_transport_stream_id transport_stream_id;
	t_original_network_id original_network_id;
	FrontendParameters feparams;
	uint8_t polarization = 0;

	/* read all transponders */
	while ((node = xmlGetNextOccurence(node, "transponder")) != NULL)
	{
		/* common */
		transport_stream_id = xmlGetNumericAttribute(node, "id", 16);
		original_network_id = xmlGetNumericAttribute(node, "onid", 16);
		feparams.Frequency = xmlGetNumericAttribute(node, "frequency", 0);
		feparams.Inversion = (SpectralInversion) xmlGetNumericAttribute(node, "inversion", 0);

		/* cable */
		if (DiSEqC == 0xFF) {
			feparams.u.qam.SymbolRate = xmlGetNumericAttribute(node, "symbol_rate", 0);
			feparams.u.qam.FEC_inner = CFrontend::getFEC(xmlGetNumericAttribute(node, "fec_inner", 0));
			feparams.u.qam.QAM = CFrontend::getModulation(xmlGetNumericAttribute(node, "modulation", 0));
		}

		/* satellite */
		else {
			feparams.u.qpsk.SymbolRate = xmlGetNumericAttribute(node, "symbol_rate", 0);
			feparams.u.qpsk.FEC_inner = CFrontend::getFEC(xmlGetNumericAttribute(node, "fec_inner", 0));
			polarization = xmlGetNumericAttribute(node, "polarization", 0);
		}

		/* add current transponder to list */
		transponders.insert
		(
			std::pair <uint32_t, transponder>
			(
				(transport_stream_id << 16) | original_network_id,
				transponder
				(
					transport_stream_id,
					feparams,
					polarization,
					DiSEqC,
					original_network_id
				)
			)
		);

		/* read channels that belong to the current transponder */
		ParseChannels(node->xmlChildrenNode, transport_stream_id, original_network_id, DiSEqC, satellite, satellitePosition);

		/* hop to next transponder */
		node = node->xmlNextNode;
	}

	return;
}

void ParseChannels(xmlNodePtr node, const t_transport_stream_id transport_stream_id, const t_original_network_id original_network_id, const unsigned char DiSEqC, std::string satellite, int32_t satellitePosition)
{
	t_service_id service_id;
	std::string  name;
	uint8_t      service_type;

	while ((node = xmlGetNextOccurence(node, "channel")) != NULL)
	{
		service_id = xmlGetNumericAttribute(node, "service_id", 16);
		name = xmlGetAttribute(node, "name");
		service_type = xmlGetNumericAttribute(node, "service_type", 16);

		switch (service_type) {
		case ST_DIGITAL_TELEVISION_SERVICE:
		case ST_NVOD_REFERENCE_SERVICE:
		case ST_NVOD_TIME_SHIFTED_SERVICE:
		case ST_DIGITAL_RADIO_SOUND_SERVICE:
			allchans.insert
			(
				std::pair <t_channel_id, CZapitChannel>
				(
					CREATE_CHANNEL_ID,
					CZapitChannel
					(
						name,
						service_id,
						transport_stream_id,
						original_network_id,
						service_type,
						DiSEqC,
						satellite, 
						satellitePosition
					)
				)
			);

			break;

		default:
			break;
		}

		node = node->xmlNextNode;
	}

	return;
}

void FindTransponder(xmlNodePtr search)
{
	uint8_t DiSEqC;
	string satellite = "None";
	int32_t satellitePosition = 0;

	while (search)
	{
		if (!(strcmp(xmlGetName(search), "cable")))
			DiSEqC = 0xff;

		else if (!(strcmp(xmlGetName(search), "sat")))
		     {
			DiSEqC = xmlGetNumericAttribute(search, "diseqc", 0);
			satellite = xmlGetAttribute(search, "name");
			satellitePosition = satellitePositions[satellite];
		     }

		else if (!(strcmp(xmlGetName(search), "terrestrial")))
			DiSEqC = 0xfe;

		else {
			search = search->xmlNextNode;
			continue;
		}

		INFO("going to parse dvb-%c provider %s", xmlGetName(search)[0], xmlGetAttribute(search, "name"));
		ParseTransponders(search->xmlChildrenNode, DiSEqC, satellite, satellitePosition);
		search = search->xmlNextNode;
	}
}

int LoadMotorPositions(void)
{
	FILE *fd = NULL;
	int motorPosition = 0;
	char buffer[256] = "";
	char pos[10] = "";
	char satellite[32] = "";
	uint i = 0;
	uint j = 0;
	
	if ((fd = fopen(MOTORCONFIGFILE, "r")))
	{
		fgets(buffer, 255, fd);
		while(!feof(fd))
		{
			i = 0; j = 0;
			while ((buffer[i] != ':') && (i < strlen(buffer)))
			{
				satellite[i] = buffer[i];
				i++;
			}
			satellite[i] = 0;
			
			i++;
			while (i < strlen(buffer))
			{
				pos[j] = buffer[i];
				i++;
				j++;
			}
			pos[j] = 0;
			
			sscanf(pos, "%d", &motorPosition);
			motorPositions[satellite]++;
			motorPositions[satellite] = motorPosition;
			fgets(buffer, 255, fd);	
		}
		fclose(fd);
	}
	std::map<std::string, uint8_t>::iterator it;
	for (it = motorPositions.begin(); it != motorPositions.end(); it++)
	{
		printf("satellite = %s, motorPosition = %d\n", it->first.c_str(), it->second);
	}
	
	return 0;
}

int LoadSatellitePositions(void)
{
	string satellite;
	int32_t satellitePosition;
	
	printf("[getservices] loading satellite positions...\n");
	xmlDocPtr parser = parseXmlFile(string(SATELLITES_XML));

	if (parser == NULL)
	{
		printf("[getservices] satellites.xml not found.\n");
		return -1;
	}

	xmlNodePtr search = xmlDocGetRootElement(parser)->xmlChildrenNode;
	while (search)
	{
		if (!(strcmp(xmlGetName(search), "sat")))
		{	
			satellite = xmlGetAttribute(search, "name");
			satellitePosition = xmlGetSignedNumericAttribute(search, "position", 10);
			satellitePositions[satellite]++;
			satellitePositions[satellite] = satellitePosition;
		}

		search = search->xmlNextNode;
	}
	
	xmlFreeDoc(parser);
	
	std::map<std::string, int32_t>::iterator it;
	for (it = satellitePositions.begin(); it != satellitePositions.end(); it++)
	{
		printf("satellite = %s, satellitePosition = %d\n", it->first.c_str(), it->second);
	}
	return 0;
}

int LoadServices(uint32_t diseqctype)
{
	if (diseqctype == DISEQC_1_2)
	{
		LoadSatellitePositions();
		LoadMotorPositions();
	}
	
	xmlDocPtr parser = parseXmlFile(string(SERVICES_XML));

	if (parser == NULL)
		return -1;

	FindTransponder(xmlDocGetRootElement(parser)->xmlChildrenNode);
	xmlFreeDoc(parser);
	return 0;
}

