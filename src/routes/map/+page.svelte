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
	import { fromLonLat, useGeographic } from 'ol/proj';
	// import WebGLTile from 'ol/layer/WebGLTile';
	// import { PMTilesRasterSource } from 'ol-pmtiles';

	let mapDiv: HTMLDivElement;
	const view = new View({
		center: fromLonLat([-100, 33]),
		zoom: 5
	});
	let map: Map;
	let background = new TileLayer({
		source: new OSM()
	});
	let alrLayer2012 = new VectorLayer({
		source: new VectorSource({
			url: '/geodata/2012.json',
			format: new GeoJSON()
		})
	});
	// let alrLayer2012old = new VectorLayer({
	// 	source: new VectorSource({
	// 		url: '/geodata/2012old.json',
	// 		format: new GeoJSON()
	// 	})
	// });

	// const rasterLayer = new WebGLTile({
	// 	source: new PMTilesRasterSource({
	// 		url: '/geodata/output.pmtiles',
	// 		// attributions: ['https://github.com/tilezen/joerd/blob/master/docs/attribution.md'],
	// 		tileSize: [512, 512]
	// 	})
	// });

	// useGeographic();

	onMount(() => {
		map = new Map({
			target: mapDiv,
			view: view
		});
		map.addLayer(background);
		map.addLayer(alrLayer2012);
		// map.addLayer(alrLayer2012old);
		// map.addLayer(rasterLayer);
	});

	export let data: PageData;
	console.log(data.msg);
</script>

<div class="w-screen h-screen">
	<div bind:this={mapDiv} class="w-full h-full" />
</div>
