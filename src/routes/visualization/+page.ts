import { error } from "@sveltejs/kit";
import type { PageLoad } from "./$types";
import { getLineFromMetadata, metadataTimezone, data2json } from "$lib/utils";


let text: string;
let items: Array<String> = [];

export const load: PageLoad = async ({ fetch }) => {
    try {
        const res = await fetch('/sqmdata/freeman/20231109_131602.dat');
        // const res = await fetch('/sqmdata/freeman/20240306_131818_4.dat');
        text = await res.text();

        items = text.split('\r\n');

        let metadata = items.filter((value) => {
            let firstLetter = value.split('')[0];
            if (firstLetter === '#') {
                return value;
            }
        }).map((value) => {
            return value.slice(2).split(',');
        });
        
        const tz = metadataTimezone(getLineFromMetadata(metadata, 'Local timezone'));
        
        let data = items.filter((value) => {
            let firstLetter = value.split('')[0];
            if (firstLetter !== '#') {
                return value;
            }
        }).map((value) => {
            return value.split(';');
            // return value.split(' ');
        });

        let dataj = data2json(data, tz);
        console.log(dataj);
        
        return {
            // sqm: JSON.stringify(data),
            // dataj: JSON.stringify(dataj),
            dataj: dataj,
            // metadata: JSON.stringify(metadata),
            metadata: metadata,
            timezone: tz
        }
    } catch (e) {
        throw error(404, "Could not find sqm file.");
    }
};