<script lang="ts">
	import { onMount } from 'svelte';
	import type { PageData } from './$types';

	import 'ol/ol.css';
	import Map from 'ol/Map';
	import View from 'ol/View';
    import GeoJSON from 'ol/format/GeoJSON';
	import OSM from 'ol/source/OSM';
    import VectorSource from 'ol/source/Vector';
	import TileLayer from 'ol/layer/Tile';
    import VectorLayer from 'ol/layer/Vector';
	import { fromLonLat } from 'ol/proj';

    // import colormap from 'colormap';
    // const min = 1;
    // const max = 200;
    // const steps = 8;
    // const ramp = colormap({
    //     colormap: 'blackbody',
    //     nshades: steps,
    // });

    // function clamp(value: Number, low: Number, high:Number) {
    //     return Math.max(low, Math.min(value, high));
    // };

    // function getColor(feature) {
    //     const popEst = feature.get('POP_EST');

    // }
    
	onMount(() => {
        const map = new Map({
            target: 'map',
			layers: [
                new TileLayer({
                    source: new OSM()
				})
			],
			view: new View({
                center: fromLonLat([-100, 33]),
				zoom: 5
			})
		});
        
        const vectorLayer = new VectorLayer({
            source: new VectorSource({
                url: '/geodata/2012.json',
                format: new GeoJSON(),
            }),
            // style: {
            //     'fill-color': ['string', ['get', 'COLOR'], '#eee'],
            // }
        });
        
        map.addLayer(vectorLayer);
	});
    
    export let data: PageData;
    console.log(data.msg);
</script>

<div class="w-screen h-screen">
	<div id="map" class="w-full h-5/6 mt-24" />
</div>
