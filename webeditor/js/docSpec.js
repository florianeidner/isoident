var standardAsker = {
								asker: Xonomy.askString
							}

var docSpec={
			onchange: function(){
				console.log("Ive been changed!")
				document.getElementById("info").innerHTML = "Konfiguration wurde geandert.";

				document.getElementById("status").style = "color:#FF0000";

				document.getElementById("status").innerHTML = "NOT SAVED";
			},
			
			validate: function(obj){
				console.log("I be validatin' now!")
			},
			
			elements: {
				"signallib": {
					menu: [{
					caption: "Add a <signal>",
					action: Xonomy.newElementChild,
					actionParameter: "<signal spn='' name='' log='' start='' len='' end='' fac='' offs='' min='' max='' type='' unit='' ddi=''/>"
					}]
				},

				"signal": {
					menu: [{
						caption: "Delete <signal>",
						action: Xonomy.deleteElement
						}],
					attributes: {
						"log": {
							asker: Xonomy.askPicklist,
							askerParameter: [
							{value: "0", caption: "Disable logging"},
							{value: "1", caption: "Enable logging"}
							]},
						"spn" : standardAsker,
						"name" : standardAsker,
						"start" : standardAsker,
						"len" : standardAsker,
						"end" : standardAsker,
						"fac" : standardAsker,
						"offs": standardAsker,
						"min": standardAsker,
						"max": standardAsker,
						"type": standardAsker,
						"unit": standardAsker,
						"ddi":standardAsker
					},
					canDropTo: ["signallib", "message"]

				},

				"messagelib": {
					menu: [{
					caption: "Add a <message>",
					action: Xonomy.newElementChild,
					actionParameter: "<message pgn='' name='' type=''/>"
					}]
				},

				"message": {
					menu: [{
						caption: "Delete <message>",
						action: Xonomy.deleteElement
						}],
					attributes: {
						"type": {
							asker: Xonomy.askPicklist,
							askerParameter: ['ISO11783','J1939']
						},
						"name": standardAsker,
						"pgn" : standardAsker,
					},
					canDropTo: ["messagelib", "device"]
				},

				"devicelib": {
					menu: [{
					caption: "Add a <device>",
					action: Xonomy.newElementChild,
					actionParameter: "<device UUID='' manufacturer='' function='' class='' industry=''/>"
					}]
				},

				"device": {
					menu: [{
						caption: "Delete <device>",
						action: Xonomy.deleteElement
						}],
					attributes: {
						"UUID" : standardAsker,
						"manufacturer" : standardAsker,
						"function": standardAsker,
						"class": standardAsker,
						"industry":standardAsker,
						"lastClaim":standardAsker,
						"lastSA":standardAsker,
						"status":standardAsker,
					}
				},

				"level1": {
					menu: [{
					caption: "Add a <mean>",
					action: Xonomy.newElementChild,
					actionParameter: "<mean name='' val='' var='false'/>"
					}],
				},

				"mean": {
					menu: [{
						caption: "Delete <mean>",
						action: Xonomy.deleteElement
						}],
					attributes: {
						"name" : standardAsker,
						"val" : standardAsker,
						"var": {asker: Xonomy.askPicklist,
								askerParameter: ['true','false']
						}
					}
				},

				"level2": {
					menu: [{
					caption: "Add a <range>",
					action: Xonomy.newElementChild,
					actionParameter: "<range name='' val=''>Enter range here.</range>"
					}]
				},

				"range": {
					menu: [{
						caption: "Delete <range>",
						action: Xonomy.deleteElement
						}],
					attributes: {
						"name" : standardAsker,
						"val" : standardAsker,
					}
				},

				"logic": {
					menu: [{
					caption: "Add a <locon>",
					action: Xonomy.newElementChild,
					actionParameter: "<locon name=''> Enter code here. </locon>"
					}]
				},

				"locon": {
					menu: [{
						caption: "Delete <locon>",
						action: Xonomy.deleteElement
						}],
					attributes: {
						"name" : standardAsker
					}
				}

			}
		};