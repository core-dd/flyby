#ifndef FLYBY_UI_H_DEFINED
#define FLYBY_UI_H_DEFINED

#include "flyby_hamlib.h"
#include <predict/predict.h>

/**
 * Entry in TLE database.
 **/
struct tle_db_entry {
	///satellite number, parsed from TLE line 1
	long satellite_number;
	///satellite name, defined in TLE file
	char name[MAX_NUM_CHARS];
	///line 1 in NORAD TLE
	char line1[MAX_NUM_CHARS];
	///line 2 in NORAD TLE
	char line2[MAX_NUM_CHARS];
	///Filename from which the TLE has been read
	char filename[MAX_NUM_CHARS];
};

/**
 * TLE database.
 **/
struct tle_db {
	///Number of contained TLEs
	int num_tles;
	///TLE entries
	struct tle_db_entry tles[MAX_NUM_SATS];
};

/**
 * Entry in transponder database.
 **/
struct sat_db_entry {
	///satellite number, for relating to TLE database
	long satellite_number;
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
	///at which day of week the transponder is turned on?
	unsigned char dayofweek[MAX_NUM_TRANSPONDERS];
	///phase something
	int phase_start[MAX_NUM_TRANSPONDERS];
	int phase_end[MAX_NUM_TRANSPONDERS];
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
 * Quits ncurses, resets the terminal and displays an error message.
 *
 * \param string Error message
 **/
void bailout(char *string);

/* This function scans line 1 and line 2 of a NASA 2-Line element
 * set and returns a 1 if the element set appears to be valid or
 * a 0 if it does not.  If the data survives this torture test,
 * it's a pretty safe bet we're looking at a valid 2-line
 * element set and not just some random text that might pass
 * as orbital data based on a simple checksum calculation alone.
 *
 * \param line1 Line 1 of TLE
 * \param line2 Line 2 of TLE
 * \return 1 if valid, 0 if not
 **/
char KepCheck(char *line1, char *line2);

/* This function updates PREDICT's orbital datafile from a NASA
 * 2-line element file either through a menu (interactive mode)
 * or via the command line.  string==filename of 2-line element
 * set if this function is invoked via the command line. Only
 * entries present within the TLE database are updated, rest
 * is ignored.
 *
 * \param string Filename, or 0 if interactive mode is to be used
 * \param tle_db Pre-loaded TLE database
 * \param orbits Parsed orbital elements to be updated along with updated TLE database. Can be set to NULL
 * \return 0 on success, -1 otherwise
 **/
int AutoUpdate(const char *string, struct tle_db *tle_db, predict_orbital_elements_t **orbits);

/**
 * Displays a menu for selecting a satellite based on their names and international designators.
 *
 * \param tle_db TLE database
 * \param orbital_elements_array Orbital elements list
 **/
int Select(struct tle_db *tle_db, predict_orbital_elements_t **orbital_elements_array);

/* This function buffers and displays orbital predictions.
 *
 * \param title Title to show on top of the screen
 * \param string Data string to display
 * \param mode Display mode for changing the UI according to the type
 * of data to show: 'p' for satellite passes, 'v' for visible passes, 's' for solar illumination prediction,
 * 'm' for moon passes, 's' for sun passes
 * \return 1 if user wants to quit, 0 otherwise
 **/
int Print(const char *title, const char *string, char mode);

/* This function acts as a filter to display passes that could
 * possibly be visible to the ground station.  It works by first
 * buffering prediction data generated by the Predict() function
 * and then checking it to see if at least a part of the pass
 * is visible.  If it is, then the buffered prediction data
 * is sent to the Print() function so it can be displayed
 * to the user.
 *
 * \param title Title to show on top of the screen
 * \param string Data to display
 * \return 1 if user wants to quit, 0 otherwise
 **/
int PrintVisible(const char *title, const char *string);

/* This function predicts satellite passes.
 *
 * \param name Name of satellite
 * \param orbital_elements Orbital elements of satellite
 * \param qth QTH at which satellite is to be observed
 * \param mode 'p' for all passes, 'v' for visible passes only
 **/
void Predict(const char *name, predict_orbital_elements_t *orbital_elements, predict_observer_t *qth, char mode);

/**
 * Convenience enum so that screens for predicting moon and sun can be unified to one function.
 **/
enum celestial_object{PREDICT_SUN, PREDICT_MOON};

/**
 * Predict passes of sun and moon, similar to Predict().
 *
 * \param object Sun or moon
 * \param qth Point of observation
 **/
void PredictSunMoon(enum celestial_object object, predict_observer_t *qth);

/* This function permits displays a satellite's orbital
 * data.  The age of the satellite data is also provided.
 *
 * \param num_orbits Number of orbits
 * \param tle_db TLE database
 * \param orbital_elements_array List of rbital elements
 **/
void ShowOrbitData(struct tle_db *tle_db, predict_orbital_elements_t **orbital_elements_array);

/**
 * Edit QTH information and save to file.
 *
 * \param qthfile File at which qth information is saved
 * \param qth Returned QTH information
 **/
void QthEdit(const char *qthfile, predict_observer_t *qth);

/* This function tracks a single satellite in real-time
 * until 'Q' or ESC is pressed.
 *
 * \param orbit_ind Which orbit is first displayed on screen (can be changed within SingleTrack using left/right buttons)
 * \param orbital_elements_array Array over orbital elements that can be tracked
 * \param qth Point of observation
 * \param transponder_db Transponder database
 * \param tle_db TLE database
 * \param rotctld rotctld connection instance
 * \param downlink_info rigctld connection instance for downlink
 * \param uplink_info rigctld connection instance for uplink
 **/
void SingleTrack(int orbit_ind, predict_orbital_elements_t **orbital_elements_array, predict_observer_t *qth, struct transponder_db *transponder_db, struct tle_db *tle_db, rotctld_info_t *rotctld, rigctld_info_t *downlink_info, rigctld_info_t *uplink_info);

/**
 * Displays information on all satellites in real-time.
 *
 * \param qth Point of observation
 * \param num_orbits Number of orbits in the orbital elements array
 * \param orbital_elements_array List of orbital elements to track
 * \param tle_db TLE database
 * \param multitype Display locator ('m') or lat/long ('l')
 * \param disttype Display units in miles ('i') or km ('k')
 **/
void MultiTrack(predict_observer_t *qth, predict_orbital_elements_t **orbital_elements_array, struct tle_db *tle_db, char multitype, char disttype);

/**
 * Display solar illumination predictions.
 *
 * \param name Name of satellite
 * \param orbital_elements Orbital elements for satellite
 **/
void Illumination(const char *name, predict_orbital_elements_t *orbital_elements);

/*
 * Print the main menu to screen.
 **/
void MainMenu();

/**
 * Display program information.
 **/
void ProgramInfo(const char *qthfile, struct tle_db *tle_db, struct transponder_db *transponder_db, rotctld_info_t *rotctld);

/**
 * Display information for new user.
 **/
void NewUser();

/**
 * Run flyby UI.
 *
 * \param new_user Whether NewUser() should be run
 * \param qthfile Write path for QTH file
 * \param observer QTH coordinates
 * \param tle_db TLE database
 * \param sat_db Transponder database
 * \param rotctld Rotctld info
 * \param downlink Downlink info
 * \param uplink Uplink info
 **/
void RunFlybyUI(bool new_user, const char *qthfile, predict_observer_t *observer, struct tle_db *tle_db, struct transponder_db *sat_db, rotctld_info_t *rotctld, rigctld_info_t *downlink, rigctld_info_t *uplink);

#endif
