<script lang="ts">
    import type { PageData } from './$types';
    
    export let data: PageData;

    console.log(data.sqm);

	// The library provides a main wrapper component
	// and a bunch empty layout components...
	// import { LayerCake, Svg, Canvas, WebGL, Html } from 'layercake';

	// import AxisX from './components/AxisX.svelte';
	// import AxisY from './components/AxisY.svelte';
	// import Line from './components/Line.svelte';
	// import Scatter from './components/Scatter.svelte';
	// import Labels from './components/Labels.svelte';

	// const data = [
	// 	{ x: 10, y: 15 },
	// 	{ x: 11, y: 25 },
	// 	{ x: 12, y: 53 }
	// ];
</script>

<div class="w-full h-[500px] border">
    <!-- <LayerCake x="x" y="y" {data}>
        <Svg>
            <Scatter fill={'red'} r={3} />
        </Svg>
    </LayerCake> -->

    <p>{ data.sqm }<p>
    

</div>

