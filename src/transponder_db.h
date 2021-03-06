#ifndef TRANSPONDER_DB_H_DEFINED
#define TRANSPONDER_DB_H_DEFINED

#include "defines.h"
#include "tle_db.h"

/**
 * Location from where satellite database entry was loaded, used in deciding which entries to write to XDG_DATA_HOME.
 **/
enum sat_db_location {
	LOCATION_NONE = (1u << 0), //not loaded from anywhere
	LOCATION_DATA_HOME = (1u << 1), //loaded from XDG_DATA_HOME
	LOCATION_DATA_DIRS = (1u << 2), //loaded from XDG_DATA_DIRS
	LOCATION_TRANSIENT = (1u << 3) //to be newly written to XDG_DATA_HOME
};

/**
 * Entry in transponder database.
 **/
struct sat_db_entry {
	///whether squint angle can be calculated
	bool squintflag;
	///attitude latitude for squint angle calculation
	double alat;
	//attitude longitude for squint angle calculation
	double alon;
	///number of transponders
	int num_transponders;
	///name of each transponder
	char transponder_name[MAX_NUM_TRANSPONDERS][MAX_NUM_CHARS];
	///uplink frequencies
	double uplink_start[MAX_NUM_TRANSPONDERS];
	double uplink_end[MAX_NUM_TRANSPONDERS];
	///downlink frequencies
	double downlink_start[MAX_NUM_TRANSPONDERS];
	double downlink_end[MAX_NUM_TRANSPONDERS];
	//where this transponder db entry is defined (bitwise or on enum sat_db_location)
	int location;
};

/**
 * Transponder database, each entry index corresponding to the same TLE index in the TLE database.
 **/
struct transponder_db {
	///number of contained satellites. Corresponds to the number of TLEs in the TLE database
	int num_sats;
	///transponder database entries
	struct sat_db_entry sats[MAX_NUM_SATS];
	///whether the transponder database is loaded, or empty
	bool loaded;
};

/**
 * Create transponder database struct.
 *
 * \return Allocated transponder database
 **/
struct transponder_db *transponder_db_create();

/**
 * Free memory associated with allocated transponder database struct.
 *
 * \param transponder_db transponder database to free
 **/
void transponder_db_destroy(struct transponder_db **transponder_db);

/**
 * Read transponder database from folders defined using the XDG file specification.
 * Database file is assumed to be located in {XDG_DATA_DIRS}/flyby/flyby.db and XDG_DATA_HOME/flyby/flyby.db.
 * A union over the files is used.
 *
 * Transponder entries defined in XDG_DATA_HOME take precedence over XDG_DATA_DIRS. XDG_DATA_DIRS
 * ordering decides precedence of entries defined across XDG_DATA_DIRS directories.
 *
 * \param tle_db Full TLE database for which transponder database entries are matched
 * \param transponder_db Returned transponder database
 **/
void transponder_db_from_search_paths(const struct tle_db *tle_db, struct transponder_db *transponder_db);

/**
 * Read transponder database from file. Only fields matching the TLE database fields are modified.
 * Transponders where neither uplink nor downlink are defined are ignored.
 *
 * \param db_file .db file
 * \param tle_db Previously read TLE database, for which fields from transponder database are matched
 * \param ret_db Returned transponder database
 * \param location_info Whether entry is being loaded from XDG_DATA_DIRS or XDG_DATA_HOME. The location flag in the loaded entries are bitwise OR-ed with the input flag
 * \return 0 on success, -1 otherwise
 **/
int transponder_db_from_file(const char *db_file, const struct tle_db *tle_db, struct transponder_db *ret_db, enum sat_db_location location_info);

/**
 * Write transponder database to file.
 *
 * All satellite database entries that are specified in the boolean array are written, irregardless of whether they are empty or not.
 *
 * Individual transponders are not written to file if neither downlink
 * nor uplink are well-defined.
 *
 * \param filename Filename
 * \param tle_db TLE database, used for obtaining name and satellite number of satellite
 * \param transponder_db Transponder database to write to file
 * \param should_write Boolean array of at least transponder_db->num_sats length. Used to specify whether a database entry should be written to file, since there are situations where we would like empty entries to be written to file (and other situations where we don't)
 **/
void transponder_db_to_file(const char *filename, struct tle_db *tle_db, struct transponder_db *transponder_db, bool *should_write);

/**
 * Write transponder database to $XDG_DATA_HOME/flyby/flyby.db. Writes only entries
 * that are marked with LOCATION_DATA_HOME or LOCATION_TRANSIENT in the `location` field.
 *
 * Entries that were not originally loaded from XDG_CONFIG_HOME, but should
 * be used to update the user database, should be marked with LOCATION_TRANSIENT or LOCATION_DATA_HOME.
 *
 * It also means that such entries will be saved to the user database irregardless
 * of whether any transponders or squint angle variables actually are defined,
 * in order to be able to override the system database.
 *
 * Entries that are empty and not defined in XDG_DATA_DIRS will not be written to file.
 *
 * Since only entries corresponding to existing TLEs will be loaded into the database,
 * only such entries will be written to the user database file. If any entries
 * without corresponding TLEs originally existed in the file, these will be overwritten.
 *
 * \param tle_db TLE database
 * \param transponder_db Transponder database to write to default location
 **/
void transponder_db_write_to_default(struct tle_db *tle_db, struct transponder_db *transponder_db);

/**
 * Check whether to satellite database entries are the same.
 *
 * \param entry_1 Entry 1
 * \param entry_2 Entry 2
 * \return True if fields in entry 1 are the same as the fields in entry 2
 **/
bool transponder_db_entry_equal(struct sat_db_entry *entry_1, struct sat_db_entry *entry_2);

/**
 * Copy contents of one satellite database entry to another.
 *
 * \param destination Destination struct
 * \param source Source struct
 **/
void transponder_db_entry_copy(struct sat_db_entry *destination, struct sat_db_entry *source);

/**
 * Check whether a transponder database entry is empty. "Empty" means that no squint angle is defined, and there are no valid transponder entries (neither uplink or downlink is defined for the transponder in question).
 *
 * \param entry Transponder database entry to check
 * \return True if transponder database entry is empty, false otherwise
 **/
bool transponder_db_entry_empty(const struct sat_db_entry *entry);

#endif
