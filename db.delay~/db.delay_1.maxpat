{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 7,
			"minor" : 2,
			"revision" : 3,
			"architecture" : "x86",
			"modernui" : 1
		}
,
		"rect" : [ 34.0, 79.0, 703.0, 760.0 ],
		"bglocked" : 0,
		"openinpresentation" : 0,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Arial",
		"gridonopen" : 1,
		"gridsize" : [ 15.0, 15.0 ],
		"gridsnaponopen" : 1,
		"objectsnaponopen" : 1,
		"statusbarvisible" : 2,
		"toolbarvisible" : 1,
		"lefttoolbarpinned" : 0,
		"toptoolbarpinned" : 0,
		"righttoolbarpinned" : 0,
		"bottomtoolbarpinned" : 0,
		"toolbars_unpinned_last_save" : 0,
		"tallnewobj" : 0,
		"boxanimatetime" : 200,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 0.0,
		"description" : "",
		"digest" : "",
		"tags" : "",
		"style" : "",
		"subpatcher_template" : "",
		"boxes" : [ 			{
				"box" : 				{
					"id" : "obj-18",
					"maxclass" : "scope~",
					"numinlets" : 2,
					"numoutlets" : 0,
					"patching_rect" : [ 437.0, 499.0, 130.0, 130.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"fontface" : 0,
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-16",
					"maxclass" : "number~",
					"mode" : 2,
					"numinlets" : 2,
					"numoutlets" : 2,
					"outlettype" : [ "signal", "float" ],
					"patching_rect" : [ 158.0, 426.0, 56.0, 22.0 ],
					"sig" : 0.0,
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-15",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "signal" ],
					"patching_rect" : [ 31.0, 343.0, 83.0, 22.0 ],
					"style" : "",
					"text" : "delay~ 44100"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-5",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 0,
					"patching_rect" : [ 467.0, 270.0, 37.0, 22.0 ],
					"style" : "",
					"text" : "dac~"
				}

			}
, 			{
				"box" : 				{
					"format" : 6,
					"id" : "obj-13",
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 286.0, 124.0, 50.0, 22.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-10",
					"maxclass" : "scope~",
					"numinlets" : 2,
					"numoutlets" : 0,
					"patching_rect" : [ 276.0, 499.0, 130.0, 130.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-9",
					"maxclass" : "scope~",
					"numinlets" : 2,
					"numoutlets" : 0,
					"patching_rect" : [ 45.0, 499.0, 130.0, 130.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"format" : 6,
					"id" : "obj-19",
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 95.0, 194.0, 67.0, 22.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-17",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 135.0, 270.0, 69.0, 22.0 ],
					"style" : "",
					"text" : "pak time 0."
				}

			}
, 			{
				"box" : 				{
					"clipheight" : 91.0,
					"data" : 					{
						"clips" : [ 							{
								"filename" : "215760.wav",
								"filekind" : "audiofile",
								"loop" : 1,
								"content_state" : 								{
									"formantcorrection" : [ 0 ],
									"originallength" : [ 0.0, "ticks" ],
									"followglobaltempo" : [ 0 ],
									"originaltempo" : [ 120.0 ],
									"basictuning" : [ 440 ],
									"mode" : [ "basic" ],
									"slurtime" : [ 0.0 ],
									"play" : [ 0 ],
									"quality" : [ "basic" ],
									"speed" : [ 1.0 ],
									"pitchcorrection" : [ 0 ],
									"originallengthms" : [ 0.0 ],
									"pitchshift" : [ 1.0 ],
									"formant" : [ 1.0 ],
									"timestretch" : [ 0 ]
								}

							}
 ]
					}
,
					"id" : "obj-4",
					"maxclass" : "playlist~",
					"numinlets" : 1,
					"numoutlets" : 5,
					"outlettype" : [ "signal", "signal", "signal", "", "dictionary" ],
					"patching_rect" : [ 467.0, 124.0, 150.0, 92.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"format" : 6,
					"id" : "obj-14",
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 356.0, 124.0, 50.0, 22.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-12",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"patching_rect" : [ 216.0, 124.0, 29.5, 22.0 ],
					"style" : "",
					"text" : "+ 1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-11",
					"maxclass" : "live.tab",
					"num_lines_patching" : 1,
					"num_lines_presentation" : 0,
					"numinlets" : 1,
					"numoutlets" : 3,
					"outlettype" : [ "", "", "float" ],
					"parameter_enable" : 1,
					"patching_rect" : [ 216.0, 92.0, 100.0, 20.0 ],
					"saved_attribute_attributes" : 					{
						"valueof" : 						{
							"parameter_longname" : "live.tab",
							"parameter_shortname" : "live.tab",
							"parameter_type" : 2,
							"parameter_enum" : [ "one", "two", "three" ],
							"parameter_unitstyle" : 0
						}

					}
,
					"varname" : "live.tab"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-8",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "signal" ],
					"patching_rect" : [ 356.0, 169.0, 65.0, 22.0 ],
					"style" : "",
					"text" : "cycle~ 0.1"
				}

			}
, 			{
				"box" : 				{
					"fontface" : 0,
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-7",
					"maxclass" : "number~",
					"mode" : 2,
					"numinlets" : 2,
					"numoutlets" : 2,
					"outlettype" : [ "signal", "float" ],
					"patching_rect" : [ 172.0, 450.0, 56.0, 22.0 ],
					"sig" : 0.0,
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"fontface" : 0,
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-6",
					"maxclass" : "number~",
					"mode" : 2,
					"numinlets" : 2,
					"numoutlets" : 2,
					"outlettype" : [ "signal", "float" ],
					"patching_rect" : [ 189.5, 474.0, 56.0, 22.0 ],
					"sig" : 0.0,
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-3",
					"maxclass" : "newobj",
					"numinlets" : 4,
					"numoutlets" : 1,
					"outlettype" : [ "signal" ],
					"patching_rect" : [ 216.0, 270.0, 229.0, 22.0 ],
					"style" : "",
					"text" : "selector~ 3 1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-2",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "signal" ],
					"patching_rect" : [ 286.0, 169.0, 33.0, 22.0 ],
					"style" : "",
					"text" : "sig~"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-1",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "signal" ],
					"patching_rect" : [ 216.0, 343.0, 93.0, 22.0 ],
					"style" : "",
					"text" : "db.delay~ 1000"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"destination" : [ "obj-10", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-1", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-7", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-1", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-12", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-11", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-3", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-12", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-2", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-13", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-8", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-14", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-16", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-15", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-9", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-15", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-17", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-15", 1 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-19", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-17", 1 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-19", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-3", 1 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-2", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-3", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-15", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-3", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-18", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-3", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-6", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-3", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-3", 3 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-4", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-3", 2 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-8", 0 ]
				}

			}
 ],
		"parameters" : 		{
			"obj-11" : [ "live.tab", "live.tab", 0 ]
		}
,
		"dependency_cache" : [ 			{
				"name" : "db.filterbank_1_20161105_7.maxsnap",
				"bootpath" : "~/Documents/Max 7/Snapshots",
				"type" : "mx@s",
				"implicit" : 1
			}
, 			{
				"name" : "db.filterbank_1_20161104_1.maxsnap",
				"bootpath" : "~/Documents/Max 7/Snapshots",
				"type" : "mx@s",
				"implicit" : 1
			}
, 			{
				"name" : "db.filterbank_1_20161105.maxsnap",
				"bootpath" : "~/Documents/Max 7/Snapshots",
				"type" : "mx@s",
				"implicit" : 1
			}
, 			{
				"name" : "db.filterbank_1_20161105_1.maxsnap",
				"bootpath" : "~/Documents/Max 7/Snapshots",
				"type" : "mx@s",
				"implicit" : 1
			}
, 			{
				"name" : "db.filterbank_1_20161105_2.maxsnap",
				"bootpath" : "~/Documents/Max 7/Snapshots",
				"type" : "mx@s",
				"implicit" : 1
			}
, 			{
				"name" : "db.filterbank_1_20161105_3.maxsnap",
				"bootpath" : "~/Documents/Max 7/Snapshots",
				"type" : "mx@s",
				"implicit" : 1
			}
, 			{
				"name" : "db.filterbank_1_20161105_4.maxsnap",
				"bootpath" : "~/Documents/Max 7/Snapshots",
				"type" : "mx@s",
				"implicit" : 1
			}
, 			{
				"name" : "db.filterbank_1_20161105_5.maxsnap",
				"bootpath" : "~/Documents/Max 7/Snapshots",
				"type" : "mx@s",
				"implicit" : 1
			}
, 			{
				"name" : "db.filterbank_1_20161105_6.maxsnap",
				"bootpath" : "~/Documents/Max 7/Snapshots",
				"type" : "mx@s",
				"implicit" : 1
			}
, 			{
				"name" : "215760.wav",
				"bootpath" : "~/Documents/Max 7/Library/MuBuForMax-1.8.10-Max-6-7/examples/mubu-pipo/overview/corpus",
				"type" : "WAVE",
				"implicit" : 1
			}
, 			{
				"name" : "db.delay~.mxo",
				"type" : "iLaX"
			}
 ],
		"autosave" : 0,
		"snapshot" : 		{
			"filetype" : "C74Snapshot",
			"version" : 2,
			"minorversion" : 0,
			"name" : "snapshotlist",
			"origin" : "jpatcher",
			"type" : "list",
			"subtype" : "Undefined",
			"embed" : 1,
			"snapshot" : 			{
				"valuedictionary" : 				{
					"parameter_values" : 					{
						"[5]" : -7.038444,
						"live.numbox" : 4.0,
						"live.numbox[1]" : 1.0,
						"live.numbox[2]" : 60.0,
						"live.tab" : 1.0,
						"live.tab[3]" : 0.0,
						"blob" : 						{
							"multislider" : [ 925, 1016, 2012, 3823 ],
							"multislider[1]" : [ 0.652, 0.652, 0.652, 0.690909 ],
							"multislider[2]" : [ 0.842, 0.507143, 0.307143, 0.114286 ],
							"multislider[3]" : [ 0.114, 0.114, 0.114, 0.114 ],
							"multislider[4]" : [ 80, 80, 80, 67 ],
							"table" : [ 925, 1016, 2012, 3823, 2646, 3370, 4366, 1740, 1831, 2465, 2510, 2736, 2917, 3370, 3732, 4366, 4819, 5000, 5000, 5000, 5000, 4819, 4638, 4412, 4185, 3944, 3702, 3461, 3144, 2827, 2601, 2374, 1921, 1770, 1620, 1469, 1016, 925, 654, 473, 292, 20, 20, 382, 654, 925, 1333, 1740, 1951, 2163, 2374, 2646, 2917, 3189, 3491, 3793, 4095, 4457, 4819, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 4940, 4879, 4819, 4457, 4095, 3914, 3732, 3416, 3099, 2918, 2736, 2555, 2374, 2103, 1921, 1650, 1469, 1378, 1288, 1197, 1016, 925, 744, 563, 382, 201, 20, 20, 111, 322, 533, 744, 948, 1152, 1355, 1559, 1831, 2102, 2374, 2601, 2827, 3054, 3280, 3521, 3763, 4004, 4185, 4366, 4547, 4774, 5000, 5000, 5000, 5000, 20, 4095 ],
							"table[1]" : [ 842, 507, 307, 114, 428, 342, 235, 662, 662, 662, 646, 642, 638, 634, 632, 629, 625, 624, 620, 614, 606, 601, 597, 592, 587, 583, 574, 564, 560, 555, 550, 546, 541, 532, 521, 492, 485, 384, 377, 370, 356, 342, 328, 314, 303, 293, 284, 277, 270, 263, 256, 250, 250, 250, 256, 256, 289, 291, 307, 310, 314, 335, 342, 351, 356, 384, 384, 418, 423, 428, 488, 507, 517, 527, 550, 546, 541, 535, 527, 509, 507, 500, 497, 492, 488, 485, 485, 485, 462, 462, 462, 462, 462, 428, 428, 409, 407, 397, 395, 391, 388, 384, 381, 377, 375, 374, 372, 370, 368, 365, 363, 358, 354, 349, 344, 337, 333, 328, 324, 319, 314, 310, 307, 296, 284, 282, 277, 671 ],
							"table[2]" : [ 652, 652, 652, 690, 453, 435, 581, 578, 634, 708, 745, 800, 851, 870, 925, 981, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 944, 888, 836, 763, 736, 634, 527, 490, 453, 435, 363, 326, 326, 326, 289, 289, 289, 289, 363, 435, 490, 545, 615, 689, 754, 818, 861, 907, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 995, 990, 989, 985, 981, 958, 935, 912, 888, 869, 847, 824, 800, 781, 757, 731, 708, 662, 615, 578, 563, 527, 509, 490, 490, 490, 490, 490, 527, 600, 629, 660, 689, 736, 781, 845, 907, 981, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 907, 671 ],
							"table[3]" : [ 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 171, 171, 228, 228, 228, 228, 228, 228, 228, 228, 228, 228, 228, 671 ]
						}

					}

				}

			}
,
			"snapshotlist" : 			{
				"current_snapshot" : 0,
				"entries" : [ 					{
						"filetype" : "C74Snapshot",
						"version" : 2,
						"minorversion" : 0,
						"name" : "db.filterbank_1",
						"origin" : "db.delay_1",
						"type" : "patcher",
						"subtype" : "Undefined",
						"embed" : 0,
						"snapshot" : 						{
							"valuedictionary" : 							{
								"parameter_values" : 								{
									"[5]" : -7.038444,
									"live.numbox" : 4.0,
									"live.numbox[1]" : 1.0,
									"live.numbox[2]" : 60.0,
									"live.tab" : 1.0,
									"live.tab[3]" : 0.0,
									"blob" : 									{
										"multislider" : [ 925, 1016, 2012, 3823 ],
										"multislider[1]" : [ 0.652, 0.652, 0.652, 0.690909 ],
										"multislider[2]" : [ 0.842, 0.507143, 0.307143, 0.114286 ],
										"multislider[3]" : [ 0.114, 0.114, 0.114, 0.114 ],
										"multislider[4]" : [ 80, 80, 80, 67 ],
										"table" : [ 925, 1016, 2012, 3823, 2646, 3370, 4366, 1740, 1831, 2465, 2510, 2736, 2917, 3370, 3732, 4366, 4819, 5000, 5000, 5000, 5000, 4819, 4638, 4412, 4185, 3944, 3702, 3461, 3144, 2827, 2601, 2374, 1921, 1770, 1620, 1469, 1016, 925, 654, 473, 292, 20, 20, 382, 654, 925, 1333, 1740, 1951, 2163, 2374, 2646, 2917, 3189, 3491, 3793, 4095, 4457, 4819, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 4940, 4879, 4819, 4457, 4095, 3914, 3732, 3416, 3099, 2918, 2736, 2555, 2374, 2103, 1921, 1650, 1469, 1378, 1288, 1197, 1016, 925, 744, 563, 382, 201, 20, 20, 111, 322, 533, 744, 948, 1152, 1355, 1559, 1831, 2102, 2374, 2601, 2827, 3054, 3280, 3521, 3763, 4004, 4185, 4366, 4547, 4774, 5000, 5000, 5000, 5000, 20, 4095 ],
										"table[1]" : [ 842, 507, 307, 114, 428, 342, 235, 662, 662, 662, 646, 642, 638, 634, 632, 629, 625, 624, 620, 614, 606, 601, 597, 592, 587, 583, 574, 564, 560, 555, 550, 546, 541, 532, 521, 492, 485, 384, 377, 370, 356, 342, 328, 314, 303, 293, 284, 277, 270, 263, 256, 250, 250, 250, 256, 256, 289, 291, 307, 310, 314, 335, 342, 351, 356, 384, 384, 418, 423, 428, 488, 507, 517, 527, 550, 546, 541, 535, 527, 509, 507, 500, 497, 492, 488, 485, 485, 485, 462, 462, 462, 462, 462, 428, 428, 409, 407, 397, 395, 391, 388, 384, 381, 377, 375, 374, 372, 370, 368, 365, 363, 358, 354, 349, 344, 337, 333, 328, 324, 319, 314, 310, 307, 296, 284, 282, 277, 671 ],
										"table[2]" : [ 652, 652, 652, 690, 453, 435, 581, 578, 634, 708, 745, 800, 851, 870, 925, 981, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 944, 888, 836, 763, 736, 634, 527, 490, 453, 435, 363, 326, 326, 326, 289, 289, 289, 289, 363, 435, 490, 545, 615, 689, 754, 818, 861, 907, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 995, 990, 989, 985, 981, 958, 935, 912, 888, 869, 847, 824, 800, 781, 757, 731, 708, 662, 615, 578, 563, 527, 509, 490, 490, 490, 490, 490, 527, 600, 629, 660, 689, 736, 781, 845, 907, 981, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 907, 671 ],
										"table[3]" : [ 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 171, 171, 228, 228, 228, 228, 228, 228, 228, 228, 228, 228, 228, 671 ]
									}

								}

							}

						}
,
						"fileref" : 						{
							"name" : "db.filterbank_1",
							"filename" : "db.filterbank_1_20161105_7.maxsnap",
							"filepath" : "~/Documents/Max 7/Snapshots",
							"filepos" : -1,
							"snapshotfileid" : "af49a840f222b24ad1ab62b71faa3a2e"
						}

					}
, 					{
						"filetype" : "C74Snapshot",
						"version" : 2,
						"minorversion" : 0,
						"name" : "db.filterbank_1",
						"origin" : "db.delay_1",
						"type" : "patcher",
						"subtype" : "Undefined",
						"embed" : 0,
						"fileref" : 						{
							"name" : "db.filterbank_1",
							"filename" : "db.filterbank_1_20161104_1.maxsnap",
							"filepath" : "~/Documents/Max 7/Snapshots",
							"filepos" : -1,
							"snapshotfileid" : "c686efb2d30cd9eb6cd750d0a20a0746"
						}

					}
, 					{
						"filetype" : "C74Snapshot",
						"version" : 2,
						"minorversion" : 0,
						"name" : "db.filterbank_1",
						"origin" : "db.delay_1",
						"type" : "patcher",
						"subtype" : "Undefined",
						"embed" : 0,
						"fileref" : 						{
							"name" : "db.filterbank_1",
							"filename" : "db.filterbank_1_20161105.maxsnap",
							"filepath" : "~/Documents/Max 7/Snapshots",
							"filepos" : -1,
							"snapshotfileid" : "a7962cb2b9728ee242a1947a6c1d4d8c"
						}

					}
, 					{
						"filetype" : "C74Snapshot",
						"version" : 2,
						"minorversion" : 0,
						"name" : "db.filterbank_1",
						"origin" : "db.delay_1",
						"type" : "patcher",
						"subtype" : "Undefined",
						"embed" : 0,
						"fileref" : 						{
							"name" : "db.filterbank_1",
							"filename" : "db.filterbank_1_20161105_1.maxsnap",
							"filepath" : "~/Documents/Max 7/Snapshots",
							"filepos" : -1,
							"snapshotfileid" : "d51c9389295b3f7381b218c5fb55ecbf"
						}

					}
, 					{
						"filetype" : "C74Snapshot",
						"version" : 2,
						"minorversion" : 0,
						"name" : "db.filterbank_1",
						"origin" : "db.delay_1",
						"type" : "patcher",
						"subtype" : "Undefined",
						"embed" : 0,
						"fileref" : 						{
							"name" : "db.filterbank_1",
							"filename" : "db.filterbank_1_20161105_2.maxsnap",
							"filepath" : "~/Documents/Max 7/Snapshots",
							"filepos" : -1,
							"snapshotfileid" : "76d7ca833a68ebf3a82cfca50d2c710a"
						}

					}
, 					{
						"filetype" : "C74Snapshot",
						"version" : 2,
						"minorversion" : 0,
						"name" : "db.filterbank_1",
						"origin" : "db.delay_1",
						"type" : "patcher",
						"subtype" : "Undefined",
						"embed" : 0,
						"fileref" : 						{
							"name" : "db.filterbank_1",
							"filename" : "db.filterbank_1_20161105_3.maxsnap",
							"filepath" : "~/Documents/Max 7/Snapshots",
							"filepos" : -1,
							"snapshotfileid" : "f945a7a2286400728b50cd6479645b75"
						}

					}
, 					{
						"filetype" : "C74Snapshot",
						"version" : 2,
						"minorversion" : 0,
						"name" : "db.filterbank_1",
						"origin" : "db.delay_1",
						"type" : "patcher",
						"subtype" : "Undefined",
						"embed" : 0,
						"fileref" : 						{
							"name" : "db.filterbank_1",
							"filename" : "db.filterbank_1_20161105_4.maxsnap",
							"filepath" : "~/Documents/Max 7/Snapshots",
							"filepos" : -1,
							"snapshotfileid" : "abb5ac34fe0a1c35ae9918b67f231111"
						}

					}
, 					{
						"filetype" : "C74Snapshot",
						"version" : 2,
						"minorversion" : 0,
						"name" : "db.filterbank_1",
						"origin" : "db.delay_1",
						"type" : "patcher",
						"subtype" : "Undefined",
						"embed" : 0,
						"fileref" : 						{
							"name" : "db.filterbank_1",
							"filename" : "db.filterbank_1_20161105_5.maxsnap",
							"filepath" : "~/Documents/Max 7/Snapshots",
							"filepos" : -1,
							"snapshotfileid" : "18228c8aeffbea5278669796b108cd26"
						}

					}
, 					{
						"filetype" : "C74Snapshot",
						"version" : 2,
						"minorversion" : 0,
						"name" : "db.filterbank_1",
						"origin" : "db.delay_1",
						"type" : "patcher",
						"subtype" : "Undefined",
						"embed" : 0,
						"fileref" : 						{
							"name" : "db.filterbank_1",
							"filename" : "db.filterbank_1_20161105_6.maxsnap",
							"filepath" : "~/Documents/Max 7/Snapshots",
							"filepos" : -1,
							"snapshotfileid" : "5eae9605be89697435cc93f25f27715c"
						}

					}
 ]
			}

		}

	}

}
