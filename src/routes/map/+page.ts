import type { PageLoad } from './$types';

export const load = (async () => {
    return {
        msg: "Hey dude, it's working!"
    };
}) satisfies PageLoad;