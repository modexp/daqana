# Set up environment variables for running modulation experiment code (in particular daqana)
#
# Original version Adam Brown, 11 Jan 2017 (abrown@physik.uzh.ch)
#
 
# You must manually set these environment variables to suit your system
########################################################
 
# Base directory (used for default locations of analysis scripts)
export MODEXP_BASE_DIR=/user/z37/Modulation
#export MODEXP_RAW_DATA_DIR=/data/modulation/Raw_data/zurich_rawdata/raw
export MODEXP_RAW_DATA_DIR=/data/modulation/Raw_data/combined
export MODEXP_PROCESSED_DATA_DIR=/data/atlas/users/acolijn/Modulation/development
#export MODEXP_PROCESSED_DATA_DIR=/dcache/xenon/tmons/Modulation/zurichdata/


# These environment variables may need changing but have sensible defaults
########################################################
 
# Location of analysis scripts from GitHub:
export MODEXP_ANALYSIS_DIR=$MODEXP_BASE_DIR/analysis
 
# Somewhere to put shell scripts used to call root scripts by daq processor
export MODEXP_TEMP_SCRIPTS_DIR=$MODEXP_BASE_DIR/stoomboot/scripts
 
# Location of calibration data.
export MODEXP_CALIBRATION_DATA_DIR=$MODEXP_PROCESSED_DATA_DIR/calibration
 
# Analysis data directory.
export MODEXP_ANALYSIS_DATA_DIR=$MODEXP_PROCESSED_DATA_DIR/analysis
