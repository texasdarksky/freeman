<script lang="ts">
	import { onMount } from 'svelte';

	import * as pmtiles from 'pmtiles';

	import 'ol/ol.css';
	import DataTile from 'ol/DataTile';
	import Map from 'ol/Map';
	import TileLayer from 'ol/layer/WebGLTile';
	import View from 'ol/View';
	import { useGeographic } from 'ol/proj';

	useGeographic();

	const tiles = new pmtiles.PMTiles(
		'https://pub-9288c68512ed46eca46ddcade307709b.r2.dev/protomaps-sample-datasets/terrarium_z9.pmtiles'
	);

	function loadImage(src: string) {
		return new Promise((resolve, reject) => {
			const img = new Image();
			img.addEventListener('load', () => resolve(img));
			img.addEventListener('error', () => reject(new Error('load failed')));
			img.src = src;
		});
	}

	async function loader(z: number, x: number, y: number) {
		const response = await tiles.getZxy(z, x, y);
		if (response !== undefined){
			const blob = new Blob([response.data]);
			const src = URL.createObjectURL(blob);
			const image = await loadImage(src);
			URL.revokeObjectURL(src);
			return image;
		}
		return;
	}

	const layer = new TileLayer({
		source: new DataTile({
			loader,
			wrapX: true,
			maxZoom: 9,
			attributions:
				"<a href='https://github.com/tilezen/joerd/blob/master/docs/attribution.md#attribution'>Tilezen Jörð</a>"
		}),
	});

	onMount(() => {
		const map = new Map({
			target: 'map',
			layers: [layer],
			view: new View({
				center: [0, 0],
				zoom: 1
			})
		});
	});
</script>
