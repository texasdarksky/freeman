// class NateDate extends Date {
//     getTimeOfDay(units: 'hours' | 'minutes' | 'seconds' | null): number {
//         let hours = (this.getSeconds() / 60 + this.getMinutes()) / 60 + this.getHours();
//         if (units === 'hours') {
//             return hours;
//         } else if (units === 'seconds') {
//             return hours * 3600;
//         } else {
//             return hours * 60;
//         }
//     }
// };

// let myDate = new NateDate('August 20, 2023 0:15:30Z');
// console.log(`${myDate.getHours()} hours, ${myDate.getMinutes()} minutes, ${myDate.getSeconds()} seconds.`)

// console.log(myDate.getTimeOfDay('minutes'));

import { DateTime } from 'luxon';
import { clsx, type ClassValue } from "clsx";
import { twMerge } from "tailwind-merge";

export function cn(...inputs: ClassValue[]) {
	return twMerge(clsx(inputs));
}

/**
 * 
 * @param metadata Array<Array<string>>
 * @param searchParam string
 * @returns string
 * @description Returns the line of metadata (from a .dat file) that matches the searchParam
 */
export function getLineFromMetadata(metadata: Array<Array<string>>, searchParam: string): string {
    let thing: string | undefined = '';

    outer: for (const inner of metadata) {
        inner: for (const item of inner) {
            if (item.includes(searchParam)) {
                thing = item;
                break outer;
            };
        };
    };

    return thing;
};

/**
 * 
 * @param metadataLine string
 * @returns string
 * @description Parses and returns the timezone string value from the given metadataLine
 * @description This function should be copied/modified to parse other metadata values
*/
export function metadataTimezone(metadataLine: string): string {
    let tz = metadataLine.split(':')[1].trim();
    // if (tz = 'CST6CDT') {
    //     tz = 'America/Chicago';
    // }
    switch (tz) {
        case 'CST6CDT':
            tz = 'America/Chicago';
            break;
        // case 'someOtherCode':
        //     tz = 'Country/City'
        //     break;
        default:
            tz = 'America/Chicago';
    }
    return tz;
};


export function data2json(data: Array<Array<string>>, timezone: string): {} {
    let obj: Array<object> = [];

    for (const line of data) {
        obj.push({
            "utc": line[0] + 'Z',
            "local-time": line[1],
            "12-hours-earlier": DateTime.fromISO(line[0] + 'Z').setZone(timezone).minus({hours: 12}),
            "hour-of-day": getTimeOfDay(DateTime.fromISO(line[0] + 'Z').setZone(timezone), 'hours'),
            "hour-of-night": getTimeOfNight(DateTime.fromISO(line[0] + 'Z').setZone(timezone), 'hours'),
            "temperature": Number(line[2]),
            "voltage": Number(line[3]),
            "msas": Number(line[4]),
            "record-type": Number(line[5])
        });
    };

    return obj;
};

/**
 * 
 * @param date DateTime
 * @param units 'hours' | 'minutes' | 'seconds' = 'minutes' 
 * @returns The time of day beginning at midnight (midnight to midnight)
 */
function getTimeOfDay(date: DateTime, units: 'hours' | 'minutes' | 'seconds' = 'minutes'): number {
    let hours = (date.get('second') / 60 + date.get('minute')) / 60 + date.get('hour');
    if (units === 'hours') {
        return hours;
    } else if (units === 'seconds') {
        return hours * 3600;
    } else {
        return hours * 60; // 'minutes' or null is the default
    }
};

/**
 * 
 * @param date DateTime
 * @param units 'hours' | 'minutes' | 'seconds' = 'minutes' 
 * @returns The time of night beginning at noon (noon to noon)
 */
function getTimeOfNight(date: DateTime, units: 'hours' | 'minutes' | 'seconds' = 'minutes'): number {
    date = date.minus({hours: 12});
    let hours = ((date.get('second') / 60 + date.get('minute')) / 60 + date.get('hour'));
    if (units === 'hours') {
        return hours;
    } else if (units === 'seconds') {
        return hours * 3600;
    } else {
        return hours * 60; // 'minutes' or null is the default
    }
};
