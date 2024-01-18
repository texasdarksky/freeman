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

function getTimeOfDay(date: Date, units: 'hours' | 'minutes' | 'seconds' = 'minutes'): number {
    let hours = (date.getSeconds() / 60 + date.getMinutes()) / 60 + date.getHours();
    if (units === 'hours') {
        return hours;
    } else if (units === 'seconds') {
        return hours * 3600;
    } else {
        return hours * 60; // 'minutes' or null is the default
    }
};

let myDate = new Date('August 20, 2023 0:15:30Z');
console.log(getTimeOfDay(myDate));
