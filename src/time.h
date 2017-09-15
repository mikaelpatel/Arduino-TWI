/**
 * @file time.h
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2017, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifndef TIME_H
#define TIME_H

struct tm {
  int8_t tm_sec;	//!< 0-59 Seconds.
  int8_t tm_min;	//!< 0-59 Minutes.
  int8_t tm_hour;	//!< 0-23 Hours.
  int8_t tm_mday;	//!< 1-31 Day in Month.
  int8_t tm_wday;	//!< 0-6 Days since Sunday.
  int8_t tm_mon;	//!< 0-11 Months since January.
  int16_t tm_year;	//!< Years since 1900.
  int16_t tm_yday;	//!< days since January 1.
  int16_t tm_isdst;	//!< Daylight Saving Time flag.

  tm() {}

  tm(int8_t wday, int16_t year, int8_t mon, int8_t mday,
     int8_t hour, int8_t min, int8_t sec) :
    tm_sec(sec),
    tm_min(min),
    tm_hour(hour),
    tm_mday(mday),
    tm_wday(wday),
    tm_mon(mon),
    tm_year(year - 1900),
    tm_yday(0),
    tm_isdst(0)
  {
  }

} __attribute__((packed));

enum {
  SUNDAY,
  MONDAY,
  TUESDAY,
  WEDNESDAY,
  THURSDAY,
  FRIDAY,
  SATURDAY
};

enum {
  JANUARY,
  FEBRUARY,
  MARCH,
  APRIL,
  MAY,
  JUNE,
  JULY,
  AUGUST,
  SEPTEMBER,
  OCTOBER,
  NOVEMBER,
  DECEMBER
};
#endif
