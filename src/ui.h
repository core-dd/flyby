#ifndef FLYBY_UI_H_DEFINED
#define FLYBY_UI_H_DEFINED

#include "hamlib.h"
#include <predict/predict.h>
#include "tle_db.h"
#include "transponder_db.h"
#include <curses.h>

/**
 * Trim whitespaces in string from end. Used for massaging output from FIELD/FORMs.
 *
 * \param string String to modify. Is modified in place
 **/
void trim_whitespaces_from_end(char *string);

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
void AutoUpdate(const char *string, struct tle_db *tle_db, predict_orbital_elements_t **orbits);

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
 * \param name Satellite name
 * \param orbital_elements Orbital elements
 **/
void ShowOrbitData(const char *name, predict_orbital_elements_t *orbital_elements);

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
 * Display transponder editor form and edit the transponder entry.
 *
 * - Transponder entry is not changed: Nothing happens. Entries from XDG_DATA_HOME remain in the user database, nothing happens to entries defined in XDG_DATA_DIRS.
 * - Transponder entry is changed: Mark with LOCATION_TRANSIENT, will be written to user database.
 * - Transponder entry is restored to system default: Is marked with LOCATION_DATA_DIRS, will not be written to user database in order to not override the system database.
 *
 * \param sat_info TLE database entry, used for getting satellite name and satellite number for later lookup for entry defined in XDG_DATA_DIRS
 * \param form_win Window to put the editor in
 * \param sat_entry Satellite database entry to edit
 **/
void EditTransponderDatabaseField(const struct tle_db_entry *sat_info, WINDOW *form_win, struct sat_db_entry *sat_entry);

/**
 * Display transponder database entry.
 *
 * \param entry Transponder database entry to display
 * \param display_window Display window to display the entry in
 **/
void DisplayTransponderEntry(struct sat_db_entry *entry, WINDOW *display_window);

/**
 * Edit entries in transponder database. Updates user database defined in XDG_DATA_HOME on exit.
 *
 * \param start_index Selected index in the menu
 * \param tle_db TLE database, used for satellite names and numbers
 * \param sat_db Satellite database to edit
 **/
void EditTransponderDatabase(int start_index, struct tle_db *tle_db, struct transponder_db *sat_db);

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
