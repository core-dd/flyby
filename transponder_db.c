#include "transponder_db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int transponder_db_from_file(const char *dbfile, const struct tle_db *tle_db, struct transponder_db *ret_db)
{
	//copied from ReadDataFiles().

	/* Load satellite database file */
	ret_db->num_sats = tle_db->num_tles;
	FILE *fd=fopen(dbfile,"r");
	long catnum;
	unsigned char dayofweek;
	char line1[80];
	int y = 0, match = 0, transponders = 0, entry = 0;
	if (fd!=NULL) {
		fgets(line1,40,fd);

		while (strncmp(line1,"end",3)!=0 && line1[0]!='\n' && feof(fd)==0) {
			/* The first line is the satellite
			   name which is ignored here. */

			fgets(line1,40,fd);
			sscanf(line1,"%ld",&catnum);

			/* Search for match */

			for (y=0, match=0; y<tle_db->num_tles && match==0; y++) {
				if (catnum==tle_db->tles[y].satellite_number)
					match=1;
			}

			if (match) {
				transponders=0;
				entry=0;
				y--;
			}

			fgets(line1,40,fd);

			if (match) {
				if (strncmp(line1,"No",2)!=0) {
					sscanf(line1,"%lf, %lf",&(ret_db->sats[y].alat), &(ret_db->sats[y].alon));
					ret_db->sats[y].squintflag=1;
				}

				else
					ret_db->sats[y].squintflag=0;
			}

			fgets(line1,80,fd);

			while (strncmp(line1,"end",3)!=0 && line1[0]!='\n' && feof(fd)==0) {
				if (entry<MAX_NUM_TRANSPONDERS) {
					if (match) {
						if (strncmp(line1,"No",2)!=0) {
							line1[strlen(line1)-1]=0;
							strcpy(ret_db->sats[y].transponder_name[entry],line1);
						} else
							ret_db->sats[y].transponder_name[entry][0]=0;
					}

					fgets(line1,40,fd);

					if (match)
						sscanf(line1,"%lf, %lf", &(ret_db->sats[y].uplink_start[entry]), &(ret_db->sats[y].uplink_end[entry]));

					fgets(line1,40,fd);

					if (match)
						sscanf(line1,"%lf, %lf", &(ret_db->sats[y].downlink_start[entry]), &(ret_db->sats[y].downlink_end[entry]));

					fgets(line1,40,fd);

					if (match) {
						if (strncmp(line1,"No",2)!=0) {
							dayofweek=(unsigned char)atoi(line1);
							ret_db->sats[y].dayofweek[entry]=dayofweek;
						} else
							ret_db->sats[y].dayofweek[entry]=0;
					}

					fgets(line1,40,fd);

					if (match) {
						if (strncmp(line1,"No",2)!=0)
							sscanf(line1,"%d, %d",&(ret_db->sats[y].phase_start[entry]), &(ret_db->sats[y].phase_end[entry]));
						else {
							ret_db->sats[y].phase_start[entry]=0;
							ret_db->sats[y].phase_end[entry]=0;
						}

						if (ret_db->sats[y].uplink_start[entry]!=0.0 || ret_db->sats[y].downlink_start[entry]!=0.0)
							transponders++;

						entry++;
					}
				}
				fgets(line1,80,fd);
			}
			fgets(line1,80,fd);

			if (match) {
				ret_db->sats[y].num_transponders=transponders;
				ret_db->loaded = true;
			}

			entry=0;
			transponders=0;
		}

		fclose(fd);
	} else {
		return -1;
	}
	return 0;
}
