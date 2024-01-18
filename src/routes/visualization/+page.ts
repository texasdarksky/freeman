import { error } from "@sveltejs/kit";
import type { PageLoad } from "./$types";

let text: string;
let items: Array<String> = [];

export const load: PageLoad = async ({ fetch }) => {
    try {
        const res = await fetch('/sqmdata/freeman/20231109_131602.dat');
        text = await res.text();

        items = text.split('\r\n');

        let newItems = items.filter((value) => {
            let firstLetter = value.split('')[0];
            if (firstLetter !== '#') {
                return value;
            }
        }).map((value) => {
            return value.split(';');
        });


        return {
            sqm: JSON.stringify(newItems)
        }
    } catch (e) {
        throw error(404, "Could not find sqm file.");
    }
};