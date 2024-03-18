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

	import * as pmtiles from 'pmtiles';
	import DataTile from 'ol/source/DataTile';
	import WebGLTileLayer from 'ol/layer/WebGLTile';

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

	useGeographic();

	const tiles = new pmtiles.PMTiles(
		'https://r2-public.protomaps.com/protomaps-sample-datasets/terrarium_z9.pmtiles'
	);

	function loadImage(src) {
		return new Promise((resolve, reject) => {
			const img = new Image();
			img.addEventListener('load', () => resolve(img));
			img.addEventListener('error', () => reject(new Error('load failed')));
			img.src = src;
		});
	};

	async function loader(z: number, x: number, y: number) {
		const response = await tiles.getZxy(z, x, y);
		const blob = new Blob([response.data]);
		const src = URL.createObjectURL(blob);
		const image = await loadImage(src);
		URL.revokeObjectURL(src);
		return image;
	};

	const rasterLayer = new WebGLTileLayer({
		source: new DataTile({
			loader,
			wrapX: true,
			maxZoom: 9,
			attributions:
				"<a href='https://github.com/tilezen/joerd/blob/master/docs/attribution.md#attribution'>Tilezen Jörð</a>"
		}),
	});

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
