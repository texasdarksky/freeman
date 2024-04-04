#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* this reads a .csv file of SQM data and calculates a new attribute that
 * measures the roughness of the SQM data  */
/* The roughness attribute is intended to allow automatic elimination of cloudy
 * data points from the SQM readings for a night; the concept is that "jaggy"
 * SQM data represents cloudy data */
/*
 * May 28, 2021
 * Using Residual Standard Error, which is traditional statistic to measure the
 * deviation of data from a regression line. RSE = SQRT (SUM of ((observed -
 * expected)**2) / (n-2)) , where n-2 is the degrees of freedom, 2 because we
 * are estimating the regression slope and y-intercept. We will plan to use RSE
 * instead of Chi-squared as previously used by Al Grauer, because RSE is
 * normalized by the number of samples and will be less dependent on the
 * half_range employed
 */

/* The input file is a .csv file that was created by the "add moon and sun"
 * option of the Unihedron UDM software; The .csv file has been edited to
 * replace slashes, dashes, "T", and colons by commas, then a location label
 * added to the beginning of each record */

/* We read one day of data at a time. There are at most 288 samples per day, so
 * arrays are dimensioned to 300, figuring 5 minute spacing and 24 x 60 minutes
 * per day; Dimensioning large numbers causes us to run out of available ram in
 * c language */

/* added calculation of number of nights since Jan 1, 2018. We have SQM data
 * begining around March, 2019, but mobile SQM data ffrom mid- 2018. So to
 * handle all possible cases later, we start counting at Jan 1, 2018 */
/* We calculate SQM data averages of second derivatives for data which satisfies
 * these requirements - the Sun is at least 18 Degrees below the horizon, and
 * the Moon is at least 10 degrees below the horizon */

/* Added calculation of RightAscension for the SQM location and each data point
 */

/* fixed to handle cases of input data not being continuous - e.g. data only
 * acquired when it's dark outside */
/* also data not on the 5 minute cardinal times */

/* added calculation of average msas value for each day when the sun is lower
 * than 18 degrees below the horizon and the moon is lower than 10 degrees below
 * the horizon */

/* Implemented the Galactic coordinates of the normal vector at the SQM location
 * The Galactic coordinates describe where in the Milky Way the SQM normal
 points.

 * inserted pi as a constant - previously it was calculated roughly by 22.0/7.0
 */

/* August 15, 2022
 * added a bug fix to eliminate a daylight savings time problem with the
 * calculation of "minutes since 3pm" */

/* January 13, 2023 - adding capability to handle data with a cadence up to 1
 * minute (sample rate) */
/* changing dimension from 300 (5-minute data) to 1500 (1-minute data) */

int yisleap(int year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int get_yday(int mon, int day, int year) {
  static const int days[2][13] = {
      {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
      {0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}};
  int days_this_year;
  int n, days_since_Jan1_2018;
  int sum = 0;

  int leap = yisleap(year);
  days_this_year = days[leap][mon] + day;

  /* count the number of days in complete previous years */
  /* note, this algorithm does not work for any dates prior to 1-1-2018 */
  for (n = year - 1; n > 2017; n = n - 1) {
    int leap = yisleap(n);
    if (leap == 0) {
      sum = sum + 365;
    } else {
      sum = sum + 366;
    }
  }

  days_since_Jan1_2018 = sum + days_this_year;
  printf(" Year=%d  Days in previous year(s)=%d\n", year, sum);
  printf(" Month=%d  Date=%d  Days this year %d\n", mon, day, days_this_year);
  return days_since_Jan1_2018;
}

double get_UT(int UTC_Hour, int UTC_Min, int UTC_Sec) {
  double UT;
  UT = UTC_Hour + (float)UTC_Min / 60. + (float)UTC_Sec / 3600.;
  printf(" in_get_UT  UT=%lf\n", UT);
  return UT;
}

double get_J2000(int year, int UTC_Month, int UTC_Day, int UTC_Hour,
                 int UTC_Min, int UTC_Sec) {
  double dwhole, dfrac, J2000_days;
  dwhole = 367 * year - (7 * (year + (UTC_Month + 9) / 12) / 4) +
           (275 * UTC_Month / 9) + UTC_Day - 730531.5;
  dfrac =
      ((double)UTC_Hour + ((double)UTC_Min) / 60. + ((double)UTC_Sec) / 3600.) /
      24.;
  J2000_days = dwhole + dfrac;
  printf(" in get_J2000  J2000_days=%lf\n", J2000_days);
  return J2000_days;
}

double get_right_ascension(int year, int UTC_Month, int UTC_Day, int UTC_Hour,
                           int UTC_Min, int UTC_Sec, double SQM_Long) {
  double right_ascension, J2000_days, UT;
  double aa, bb, cc, dd, ee;
  int multiples;

  /* need to calculate Universal Time (UT) and J2000_day */
  /* set up test data for which we know the answer
      year = 2008;
      UTC_Month = 4;
      UTC_Day = 4;
      UTC_Hour = 15;
      UTC_Min = 30;
      UTC_Sec = 0;
      SQM_Long = -1.9166667;
  */
  printf(" in get_right_ascension -- SQM_Long=%g\n", SQM_Long);

  /* get the J2000 day value */
  J2000_days = get_J2000(year, UTC_Month, UTC_Day, UTC_Hour, UTC_Min, UTC_Sec);
  printf(" J2000_days=%g\n", J2000_days);

  /*  get the Universal time as a fraction of a day */
  UT = get_UT(UTC_Hour, UTC_Min, UTC_Sec);
  printf(" UT=%g\n", UT);

  right_ascension = 100.46 + 0.985647 * J2000_days + SQM_Long + 15. * UT;
  printf(" right_ascension before 0-360 check(degrees) =%g\n", right_ascension);

  /* make sure that the value is within the range of 0 to 360 degrees */
  /* how many multiples of 360 do we have?  Subtract or add out that number */
  /* note that multiples is an integer and that the remainder is truncated in
   * the next calculation */
  multiples = right_ascension / 360.;
  if (multiples > 0) {
    right_ascension = right_ascension - (float)multiples * 360.;
  }
  if (multiples < 0) {
    right_ascension = right_ascension - (float)multiples * 360.;
  }
  if (right_ascension < 0) {
    right_ascension = right_ascension + 360.;
  }
  printf(" right_ascension after 0-360 check(degrees) =%g\n", right_ascension);

  /* convert right_ascension from degrees to hours */
  right_ascension = right_ascension / 15.;

  printf(" right_ascension (hours) =%g\n", right_ascension);
  return right_ascension;
}

int main(int argc, char *argv[]) {
  int i = 0, j = 0, k = 0, m = 0;
  int minutes_since_3pm[1500];
  int dUYear[1500], dUMonth[1500], dUDay[1500], dUHour[1500], dUMinute[1500];
  float dUSeconds[1500];
  int dYear[1500], dMonth[1500], dDay[1500], dHour[1500], dMinute[1500];
  float dSeconds[1500];
  float dMsas[1500], dMsas_Corr[1500];
  float dVolts[1500], dCelsius[1500];
  float dMoonPhase[1500], dMoonElev[1500], dMoonIllum[1500], dSunElev[1500];
  float msas_Avg[1500], msas_Sum, msas_Count;
  int dStatus[1500];
  char NameIn[120];
  char NameOut[120];
  char SQM_Location[30];
  char blank[200];
  int nfile, length, ret, Start, Last, len2;
  double right_ascension, SQM_Lat, SQM_Long, SQM_Dec, SQM_RA, J2000_days;
  int timediff, num_minutesA, num_minutesB;
  float remainder;
  double Galactic_Lat, Galactic_Long;
  double pi;
  double XX, YY;

  /* NGP is North Galactic Pole, NCP is North Celestial Pole */
  double RightAscension_NGP, Dec_NGP, Galactic_Long_NCP;
  long double sum_x, sum_y, sum_xy, sum_x2, sum_y2, N, mean_x, mean_y, mean_xy,
      mean_x2;
  long double slope, yintercept, rcorr, rsqrd;
  int half_range, kk, timediff_max;
  long double Observed[1500], Expected[1500], nodata1, nodata2;
  long double DOF, RSE[1500], SS[1500], RSE_mult;

  /* added to handle the daylight savings time fix to "minutes since 3pm" */
  int dPosNeg, dHour_Delta, dShift_Hour;

  /* set up a multiplier for the RSE values and no-data output values */
  /* we output the RSE values multiplied by this constant to give more
   * manageable values */
  RSE_mult = 1000.;
  nodata1 = 999. * RSE_mult;
  nodata2 = 888. * RSE_mult;

  /* specify the maxiumum number of minutes allowed between SQM samples, prior
   * to marking a data gap */
  timediff_max = 16.;

  /* Run this program by specifying the program name, followed by three
   * parameters: 1) A file of SQM data which has already been processed as a csv
   * with sun and moon data */
  /*                      2) The latitude of the SQM location in fractions and
   *                      3) The longitude of the SQM location in fractions and
   * so the command line should look like this:
   *                             ./addSQMattributes inputfilename.csv 43.7916667
   * -120.23422 */

  printf("We are running Program %s\n", argv[0]);

  if (argc != 5) {
    printf(" You need to supply four parameters, the name of an input .csv "
           "file, the lat and long of the SQM and the Half Range for "
           "Chi-Squared Calc \n");
    printf(" The command line should look something like this: "
           "./addSQMattributes inputfilename.csv 43.7916667 -120.23422 9\n");
    goto Termination;
  }

  len2 = strlen(argv[1]);
  /* printf(" The input csv filename on reading is: -%s- which has %d
   * characters. \n", argv[1], len2-1);*/
  strcpy(NameIn, argv[1]);
  printf(" The input csv filename is: %s\n", NameIn);

  /* printf(" The latitude of the SQM on reading is: %s\n", argv[2]); */
  sscanf(argv[2], "%lf", &SQM_Lat);
  printf(" The latitude of the SQM is: %lf\n", SQM_Lat);

  /* printf(" The longitude of the SQM on reading is: %s\n", argv[3]); */
  sscanf(argv[3], "%lf", &SQM_Long);
  printf(" The longitude of the SQM is: %lf\n", SQM_Long);

  /* printf(" The Half Range value on reading is: %d\n", argv[4]); */
  sscanf(argv[4], "%d", &half_range);
  printf(" The Half Range is: %d\n", half_range);

  /* Open the input file */

  /* printf("\n About to open the Input Data File");       */

  FILE *fdata = fopen(NameIn, "r");
  if (fdata == NULL) {
    printf("\n Failed to open the Data File \n");
    return -1;
  }
  /* printf("\n Opened the Input Data File \n");       */

  /* Open an output file to hold the output data */
  /* tack on "SQM_attr" before the .csv */

  strcpy(NameOut, NameIn);
  strncat(NameOut, "_SQM_Attr3.csv", 15);

  printf("\n The Output Data Filename is %s \n", NameOut);

  FILE *fdataout = fopen(NameOut, "w");
  if (fdataout == NULL) {
    printf("\n Failed to open the Output Data File \n");
    return -1;
  }

  /* printf("\n Opened the Output Data File \n");       */

  /* half_range is the number of samples (usually 5 minutes apart) to include
   * before and after the current point at which the Residual Standard Error of
   * regression calculation is performed; the full width of the interval in
   * terms of number of points to consider in the calculation is (2*half_range +
   * 1) so that a half_range of 6 incorporates a total range of 60 minutes; note
   * that we don't increment by one in the minutes calculation for the middle
   * point in the time range because it is already taken into account half_range
   * of 9 evaluates to a 90 minute range
   */
  N = (long double)((2 * half_range) + 1.);

  printf(" \n");
  printf(" The half_range parameter is set to: %d\n", half_range);
  printf(" This means that the Residual Error calculation operates over %d "
         "samples\n",
         (int)N);
  printf(" In other words, if the sample spacing is 1 minute, then the range "
         "is %d minutes.\n",
         (int)half_range * 2 * 1);
  printf("                 if the sample spacing is 5 minutes, then the range "
         "is %d minutes.\n",
         (int)half_range * 2 * 5);
  printf(" Or              if the sample spacing is 15 minutes, then the range "
         "is %d minutes.\n",
         (int)half_range * 2 * 15);
  printf(" \n");
  printf(" \n");
  printf(" Residual Standard Error values that we output are multiplied by %d "
         "to achieve larger values.\n",
         (int)RSE_mult);
  printf(" \n");
  printf(" We allow gaps of %d minutes between SQM samples prior to marking a "
         "data gap.\n",
         timediff_max);
  printf(" \n");

  /* Write a header record to the output file */

  fprintf(fdataout,
          "Location,Lat,Long,UTC_Date,UTC_Time,Local_Date,Local_Time,Celsius,"
          "Volts,Msas,Status,MoonPhase,MoonElev,MoonIllum,SunElev,"
          "MinSince3pmStdTime,Msas_Avg,NightsSince_1118,RightAscensionHr,"
          "Galactic_Lat,Galactic_Long,J2000days,ResidStdErr\n");

  /* set up some constant values used later to calculate the Galactic
   * Coordinates of the normal at the SQM location */
  /* These are from the Wikipedia Celestial coordinate system  */
  pi = 3.14159265359;

  /* convert these constants from degrees to radians */

  RightAscension_NGP = 192.85948 * (pi) / 180.;
  printf("RightAscension_NGP=%lf \n", RightAscension_NGP);

  Dec_NGP = 27.12825 * (pi) / 180.;
  printf("Dec_NGP=%lf \n", Dec_NGP);

  Galactic_Long_NCP = 122.93192 * (pi) / 180.;
  printf("Galactic_Long_NCP=%lf \n", Galactic_Long_NCP);

  /* Read the data file */
  /* Read the first header record and throw it away */
  /* Note that the string read format statement, reads up to the first carriage
   * return in the input file, then reads the carriage return itself */

  fscanf(fdata, "%[^\n]\n", blank);

  /* initiate the record counter */
  m = -1;

  /* initiate the flag on 15 hundred hour */
  Start = 0;

/* increment the counter */
ReadAnother:
  m = m + 1;
  /* printf("m=%d \n", m); */
  if (m > 1499) {

    printf("We have more than 1500 samples for this day.\n");
    printf("If this is good data, sorry but this option does not handle a data "
           "cadence smaller than 1 minute. \n");
    printf("Alternately, does the input file have bad data? \n");
    printf("  Was the SQM battery dying and taking a sample too frequently or "
           "off schedule?\n");
    printf("  The input data are therefore suspect.\n");
    printf("Premature end of processing! \n");
    goto Termination;
  }

  /* Note that the string read format statement listed first, reads up to the
   * first comma in the input file */
  ret = fscanf(
      fdata,
      "%[^,],%d,%d,%d,%d,%d,%f,%d,%d,%d,%d,%d,%f,%f,%f,%f,%d,%f,%f,%f,%f\n",
      SQM_Location, &dUYear[m], &dUMonth[m], &dUDay[m], &dUHour[m],
      &dUMinute[m], &dUSeconds[m], &dYear[m], &dMonth[m], &dDay[m], &dHour[m],
      &dMinute[m], &dSeconds[m], &dCelsius[m], &dVolts[m], &dMsas[m],
      &dStatus[m], &dMoonPhase[m], &dMoonElev[m], &dMoonIllum[m], &dSunElev[m]);

  /* if we reach the end of the input file, proceed to write out the data of the
   * last day before terminating */
  if (ret == EOF) {
    goto LastDay;
  }

  printf("fscanf returned %d  m=%d \n", ret, m);
  if (ret < 21) {
    /* if here, the data record was short of values and therefore considered
     * bad. Skip this point and read another; first decrement m */
    m = m - 1;
    goto ReadAnother;
  }

  /* How long is the string in SQM_Location */
  length = strlen(SQM_Location);
  /* printf(" string -%s- length = %d\n", SQM_Location, length); */

  /*  Calculate the number of minutes since Local time 3PM for the time
   * associated with this SQM record, */
  /*  implement a bug fix to eliminate a problem with daylight savings time. Use
   * the UTC time values and correct the UTC via the longitude of the sample.
   * Previously the Local Time was used, which jumped an hour at the Daylight
   * Savings Time change */
  /*  This is the old code, commented out:
          if(dHour[m] > 14){
             minutes_since_3pm[m] = (dHour[m] -15) *60 + dMinute[m] +
     (int)(dSeconds[m]/60.+0.5);
          }
          else
          {
             minutes_since_3pm[m] = 540 + dHour[m] *60 + dMinute[m] +
     (int)(dSeconds[m]/60.+0.5);
          }

  */
  /* printf(" m=%d dHour=%d Minute=%d Seconds=%f minutes_since_3pm=%d \n", m,
   * dHour[m], dMinute[m], dSeconds[m], minutes_since_3pm[m]);*/

  /* new code follows */
  dPosNeg = 1;
  if (SQM_Long < 0.0) {
    dPosNeg = -1;
  }

  /* assignment to an integer will cause truncation of the remainder in the
   * following statement, as desired */
  dHour_Delta = abs(SQM_Long) / 15. * dPosNeg;
  dShift_Hour = dUHour[m] + dHour_Delta;

  printf(" dPosNeg= %d\n", dPosNeg);
  printf(" dHour_Delta= %d\n", dHour_Delta);
  printf(" dShift_Hour= %d\n", dShift_Hour);

  if (dShift_Hour > 14) {
    minutes_since_3pm[m] =
        (dShift_Hour - 15) * 60 + dUMinute[m] + (int)(dUSeconds[m] / 60. + 0.5);
  } else {
    minutes_since_3pm[m] =
        540 + dShift_Hour * 60 + dUMinute[m] + (int)(dUSeconds[m] / 60. + 0.5);
  }

  /* check whether we have reached a gap in the input data time - i.e. is this
   * data point more than the specified maximum gap length in minutes beyond the
   * last data point? */
  if (m > 0) {
    /* calculate the number of minutes associated with the current data point
     * time, and compare with the previous point */
    /* handle the special case of crossing the midnight boundary */
    if (dDay[m] == dDay[m - 1]) {

      /* if here, this new point is on the same day */
      num_minutesA = (int)(dHour[m] * 60. + dMinute[m]);
    } else {

      /* if here, we have crossed the midnight boundary */
      num_minutesA = (int)(24. * 60. + dMinute[m]);
    }

    num_minutesB = (int)(dHour[m - 1] * 60. + dMinute[m - 1]);
    timediff = num_minutesA - num_minutesB;
    /* make sure timediff is positive */
    if (timediff < 0) {
      timediff = timediff * -1;
    }

    if (timediff > timediff_max) {
      /* if here, we have found a time gap in the data - consider the data so
       * far for this day to be all that there is */
      printf("Found a %d minute gap in the data just after %d-%d-%d %d:%d:%d\n",
             timediff, dYear[m - 1], dMonth[m - 1], dDay[m - 1], dHour[m - 1],
             dMinute[m - 1], (int)dSeconds[m - 1]);
      /* handle the case of a patch of data after a data gap during the daytime
       * and prior to 15:00.  */
      if (dHour[m] < 15) {
        /* set Start flag to 3, which we check later to loop appropriately */
        Start = 3;
      }
      /* so jump into the loop which calculates the second derivatives, etc and
       * writes out the data to the output file for the data prior to this data
       * gap */
      goto LastDay;
    }
  }

  /* reset the Start flag if we are already past the first day of data and if we
   * have gone beyond the 15 hundred hour */
  if (Start == 2) {
    if (dHour[m] > 15) {
      Start = 0;
    }
  }

  /* reset the Start flag if we are already past the first day of data and if we
   * have reached 15 hundred hour */
  /* for the case of a partial day due to a data gap prior to 15:00 */
  if (Start == 3) {
    if (dHour[m] == 15) {
      Start = 0;
    }
  }

  /* We copy data for each day into memory, beginning at 15:00 hours and going
   * all evening, night, morning to the next afternoon */
  /* We use a flag called Start to handle reading all the other samples acquired
   * at the beginning of the day, samples which were acquired when the hour was
   * still 15. */

  /* Check to see if we reached 15:00 hours on this day; we assume that the data
   * are ordered in time sequence */
  if (dHour[m] == 15 && Start == 0) {
  /* the last sample of the previous day was m-1, so we know that the previous
   * day has values in the arrays from 0 to m-1 */
  LastDay:
    Last = m - 1;

    /* Calculation of average Msas for the day; */
    /* loop on the day's data */
    msas_Sum = 0.0;
    msas_Count = 0.0;
    for (k = 0; k < Last + 1; k++) {

      /* Sun is lower than 18 degrees below the horizon and the moon is lower
       * than 10 degrees below the horizon */
      if (dSunElev[k] < -18.0 && dMoonElev[k] < -10.0) {

        /* tally sum and count for msas average */
        msas_Sum = msas_Sum + dMsas[k];
        msas_Count = msas_Count + 1.0;
      }
    }
    /* we will later print out the average value for this day in those records
     * that contributed to the average, not to all records */
    /* That is, we will print out the average value for this day for those
     * records when the sun was less than 18 degree below the horizon*/
    /* We assign a null value (-1.0) to all points of the sun higher than -18
     * degrees, and all points lacking any count values */

    for (k = 0; k < Last + 1; k++) {

      if (dSunElev[k] < -18.0 && dMoonElev[k] < -10.0) {

        /* handle case of no values in the msas sum */
        msas_Avg[k] = -1.0;
        if (msas_Count > 0.0) {
          msas_Avg[k] = msas_Sum / msas_Count;
        }
      } else {
        msas_Avg[k] = -1.0;
      }
    }

    /* Calculate Residual Standard Error values - samples are assumed to be a
     * constant number of minutes apart; Set half_range at the program command
     * line to specify the number of samples to consider, and given the spacing
     * between SQM measurements, the number of minutes in the sample range;
     * This fits a regression line to each point of data, and with half_range
     * set to 9 and 5-minute sample spaceing then you get a range from 45
     * minutes before to 45 minutes after the point, for a total of 90 minutes;
     * Program calculates the deviation from the straight line, * expressed by
     * the sum of ((observed - expected)**2 /(expected)).
     */

    /* initialize SS and RSE array values to zero */
    /* use kk to track array elements of array */
    for (kk = 0; kk < 1500; kk++) {
      SS[kk] = 0.0;
    }
    for (kk = 0; kk < 1500; kk++) {
      RSE[kk] = 0.0;
    }

    /* set the first half_range of RSE values a giant value ; ditto for the last
     * half_range values */
    for (kk = 0; kk < half_range; kk++) {
      RSE[kk] = nodata1;
    }
    for (kk = Last + 1 - half_range; kk < m; kk++) {
      RSE[kk] = nodata1;
    }

    /* set up the degrees of freedom; we estimate two parameters, the linear
     * regression slope and y-intercept */
    DOF = (long double)((half_range * 2) + 1 - 2);

    /* loop on the sample point about which we calculate the statistic */

    /* first check to see if we have enough points in the current day to
     * calculate a valid standard error statistic */
    /* N is the number of point in the range of the standard error calculation
     */
    if (m < N) {
      printf("We only have %d data points for this day/segment and can't "
             "calculate a valid standard error. \n",
             m);
    } else {
      for (kk = half_range; kk < Last + 1 - half_range; kk++) {
        /* initialize sums */
        sum_x = 0.0;
        sum_y = 0.0;
        sum_xy = 0.0;
        sum_x2 = 0.0;
        sum_y2 = 0.0;
        RSE[kk] = 0.0;
        SS[kk] = 0.0;

        /* loop across the 2*half_range +1 values and tabulate statistics */
        for (k = kk - half_range; k < kk + half_range + 1; k++) {
          sum_x = sum_x + (long double)minutes_since_3pm[k];
          sum_y = sum_y + (long double)dMsas[k];
          sum_xy = sum_xy +
                   (long double)minutes_since_3pm[k] * (long double)dMsas[k];
          sum_x2 = sum_x2 + (long double)minutes_since_3pm[k] *
                                (long double)minutes_since_3pm[k];
          sum_y2 = sum_y2 + (long double)dMsas[k] * (long double)dMsas[k];
          /* printf("k= %d   sum_x=%Lf  sum_y=%Lf \n", k, sum_x, sum_y); */
        }
        /* calculate means */
        mean_x = sum_x / N;
        mean_y = sum_y / N;
        mean_xy = sum_xy / N;
        mean_x2 = sum_x2 / N;
        /* calculate the slope and Y-intercept of the regression line */
        slope = (mean_xy - (mean_x * mean_y)) / (mean_x2 - (mean_x * mean_x));
        yintercept = ((mean_x2 * mean_y) - (mean_xy * mean_x)) /
                     (mean_x2 - (mean_x * mean_x));
        rcorr =
            (sum_xy - sum_x * sum_y / N) / sqrt((sum_x2 - (sum_x * sum_x) / N) *
                                                (sum_y2 - (sum_y * sum_y) / N));
        rsqrd = rcorr * rcorr;
        /* printf("kk = %d   slope=%Lf  yintercept=%Lf \n", kk, slope,
         * yintercept); */

        /* Evaluate the regression line at all points of this interval of data
         * to get the expected values (on the regression line) and use the
         * observed and expected values in the Residual Standard Error (RSE)
         * calculation
         */
        for (k = kk - half_range; k < kk + half_range + 1; k++) {
          Expected[k] = slope * (long double)minutes_since_3pm[k] + yintercept;
          Observed[k] = (long double)dMsas[k];
          SS[kk] = SS[kk] +
                   ((Observed[k] - Expected[k]) * (Observed[k] - Expected[k]));
        }

        /* note that we use sqrtl here, which takes a long double argument */
        RSE[kk] = (sqrtl(SS[kk] / DOF)) * RSE_mult;

        /* fix up for any negative values because Expected is negative */
        if (RSE[kk] < 0.0) {
          RSE[kk] = RSE[kk] * -1.0;
        }

        /* fix up for any "not a number" for the case of divide by zero above */
        if (isnan(RSE[kk])) {
          RSE[kk] = nodata2;
        }

        printf("kk = %d  RSE=%Lf\n", kk, RSE[kk]);
      }
    }

    /* now print all this day's records to the output file */

    for (k = 0; k < Last + 1; k++) {

      /* Calculate a new variable - the number of days since Jan 1, 2018 */
      int days = get_yday(dMonth[k], dDay[k], dYear[k]);

      /* We actually want the number of nights since Jan 1, 2018 - that is we
       * want to count the evening and night as part of the same "day" -
       * actually the same "night"; So if the minutes since 15:00 hours is
       * greater than 540 (i.e. after midnight) we subtract one day from the
       * "days" value so those times are considered part of the previous day
       * (i.e."night") */
      /*
         Bug fix April 29, 2023 - the previous algorithm, commented out just
       below, did not work during daylight savings time.
       * Fixed the problem by first checking to see if the local hour is as
       expected, give the longitude of the SQM site.
       * We expect the local hour to be longitude/15 off from the UTC hour,
       which is the case during non-daylight savings time.
       * If the local hour is not as expected, then instead of shifting at 540
       minutes (midnight), we shift at 480 minutes to
       * provide a consistent "nights since 1118" attribute */

      /*              if(minutes_since_3pm[k] >= 540) {
                         days = days -1;
                      }
      */
      /* new code follows */
      /* a check shows that this new algortihm is not working - needs to study
       * this further */
      dPosNeg = 1;
      if (SQM_Long < 0.0) {
        dPosNeg = -1;
      }

      /* assignment to an integer will cause truncation of the remainder in the
       * following statement, as desired */
      dHour_Delta = abs(SQM_Long) / 15. * dPosNeg;
      dShift_Hour = dUHour[k] + dHour_Delta;
      if (dShift_Hour == dHour[k]) {
        /* if here, we are in not in Daylight Savings Time */
        if (minutes_since_3pm[k] >= 540) {
          days = days - 1;
        }
      } else {
        /* if here, we are in Daylight Savings Time */
        if (minutes_since_3pm[k] >= 480) {
          days = days - 1;
        }
      }

      /* calculate right ascension for the SQM_Location */
      right_ascension =
          get_right_ascension(dUYear[k], dUMonth[k], dUDay[k], dUHour[k],
                              dUMinute[k], (int)dUSeconds[k], SQM_Long);
      printf(" right_ascension=%10.6lf\n", right_ascension);

      /* convert right_ascension (SQM_RA) from hours to radians */
      SQM_RA = (right_ascension * 15.) * (pi / 180.);

      /* the Declination of the SQM is its Latitude, convert it from decimal
       * degrees to radians */
      SQM_Dec = SQM_Lat * (pi / 180.);

      /* the following Equations are from Wikipedia on Celestial Coordinate
       * Systems */
      /* we previously set up these constants:    RightAscension_NGP,  Dec_NGP,
       * Galactic_Long_NCP */

      Galactic_Lat =
          asin(sin(SQM_Dec) * sin(Dec_NGP) +
               cos(SQM_Dec) * cos(Dec_NGP) * cos(SQM_RA - RightAscension_NGP));
      /*              Galactic_Long =  Galactic_Long_NCP - ( asin( (cos(SQM_Dec)
       * * sin(SQM_RA-RightAscension_NGP)/cos(Galactic_Lat)) ) ); */
      YY = cos(SQM_Dec) * sin(SQM_RA - RightAscension_NGP);
      XX = (sin(SQM_Dec) * cos(Dec_NGP)) -
           (cos(SQM_Dec) * sin(Dec_NGP) * cos(SQM_RA - RightAscension_NGP));
      Galactic_Long = Galactic_Long_NCP - atan2(YY, XX);

      /* convert Galactic_Lat and Galactic_Long from radians to degrees */
      Galactic_Lat = Galactic_Lat * (180. / pi);
      Galactic_Long = Galactic_Long * (180. / pi);

      /* Make sure that Galactic_Long is a positive number */
      if (Galactic_Long < 0.0) {
        Galactic_Long = 360. + Galactic_Long;
      }

      /*  get the J2000 day value */
      J2000_days = get_J2000(dUYear[k], dUMonth[k], dUDay[k], dUHour[k],
                             dUMinute[k], (int)dUSeconds[k]);

      printf("%s,%12.7lf,%12.7lf,%04d-%02d-%02d,%02d:%02d:%02d,%04d-%02d-%02d,%"
             "02d:%02d:%02d,%.1f,%.2f,%.2f,%1d,%.1f,%.3f,%.1f,%.3f,%04d,%f,%"
             "04d,%12.7lf,%12.7lf,%10.5lf,%lf,%Lf\n",
             SQM_Location, SQM_Lat, SQM_Long, dUYear[k], dUMonth[k], dUDay[k],
             dUHour[k], dUMinute[k], (int)dUSeconds[k], dYear[k], dMonth[k],
             dDay[k], dHour[k], dMinute[k], (int)dSeconds[k], dCelsius[k],
             dVolts[k], dMsas[k], dStatus[k], dMoonPhase[k], dMoonElev[k],
             dMoonIllum[k], dSunElev[k], minutes_since_3pm[k], msas_Avg[k],
             days, right_ascension, Galactic_Lat, Galactic_Long, J2000_days,
             RSE[k]);

      /* Note, we need to output two numbers for each of hour, minute and
         seconds. If only one digit is output, Spotfire, and other programs,
         will take the digit as a ten's value, insted of a one's value*/
      fprintf(fdataout,
              "%s,%12.7lf,%12.7lf,%04d-%02d-%02d,%02d:%02d:%02d,%04d-%02d-%02d,"
              "%02d:%02d:%02d,%.1f,%.2f,%.2f,%1d,%.1f,%.3f,%.1f,%.3f,%04d,%f,%"
              "04d,%12.7lf,%12.7lf,%10.5lf,%lf,%Lf\n",
              SQM_Location, SQM_Lat, SQM_Long, dUYear[k], dUMonth[k], dUDay[k],
              dUHour[k], dUMinute[k], (int)dUSeconds[k], dYear[k], dMonth[k],
              dDay[k], dHour[k], dMinute[k], (int)dSeconds[k], dCelsius[k],
              dVolts[k], dMsas[k], dStatus[k], dMoonPhase[k], dMoonElev[k],
              dMoonIllum[k], dSunElev[k], minutes_since_3pm[k], msas_Avg[k],
              days, right_ascension, Galactic_Lat, Galactic_Long, J2000_days,
              RSE[k]);
    }

    /* if we are at the EOF, we have already written out the last day's data, so
     * terminate */
    if (ret == EOF) {
      goto Termination;
    }

    /* if here, we have written out all of the day's attributes, so keep the
     * very last record and proceed to read the next record */
    dUYear[0] = dUYear[m];
    dUMonth[0] = dUMonth[m];
    dUDay[0] = dUDay[m];
    dUHour[0] = dUHour[m];
    dUMinute[0] = dUMinute[m];
    dUSeconds[0] = dUSeconds[m];
    dYear[0] = dYear[m];
    dMonth[0] = dMonth[m];
    dDay[0] = dDay[m];
    dHour[0] = dHour[m];
    dMinute[0] = dMinute[m];
    dSeconds[0] = dSeconds[m];
    dCelsius[0] = dCelsius[m];
    dVolts[0] = dVolts[m];
    dMsas[0] = dMsas[m];
    dStatus[0] = dStatus[m];
    dMoonPhase[0] = dMoonPhase[m];
    dMoonElev[0] = dMoonElev[m];
    dMoonIllum[0] = dMoonIllum[m];
    dSunElev[0] = dSunElev[m];
    minutes_since_3pm[0] = minutes_since_3pm[m];
    RSE[0] = RSE[m] / RSE_mult;

    /* m is incremented above, so set it to zero here; this avoids writing over
     * the data we just stored at location zero */
    m = 0;

    if (Start == 3) {
      /* if here, we have a case of a partial day of data after a data gap and
       * prior to 15:00 in the day */
      goto ReadAnother;
    } else {

      /* set the flag to direct all the next samples acquired in the "15"
       * hundred hour into a new day */
      Start = 2;
    }
    goto ReadAnother;
  }
  goto ReadAnother;

/* if here, we have reached the EOF on fscanf */
Termination:
  printf(" Reached the End of File");
  fclose(fdata);
  fclose(fdataout);
}
