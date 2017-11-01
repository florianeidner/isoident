function load_configfile() {


	$.ajax({
	    type: "GET" ,
	    url: "/isoident.xml" ,
	    dataType: "xml" ,
	    success: function(xml) {

	    	Xonomy.setMode("nerd");

			var editor=document.getElementById("editor");
		
			Xonomy.render(xml, editor, docSpec);

			document.getElementById("info").innerHTML = "isoident.xml erfolgreich geladen.";

			document.getElementById("status").style = "color:#008000";

			document.getElementById("status").innerHTML = "SAVED";

	    },

	    error: function(jqXHR, exception) {
        	console.log(jqXHR);

        	document.getElementById("info").innerHTML = "Konfiguration konnte nicht geladen werden.";

			document.getElementById("status").style = "color:#FF0000";
	
			document.getElementById("status").innerHTML = "FAILED";;
        	
        }



	});
}

function save_configfile() {
	var xmlContent = Xonomy.harvest();
	
	 $.ajax({  
        type: 'POST',

        url: '/webeditor/php/saveXML.php',          

        data: {xml : xmlContent},

        success: function(response) {
            
            console.log(response);   
        	
        	document.getElementById("info").innerHTML = "isoident.xml erfolgreich gespeichert.";

			document.getElementById("status").style = "color:#008000";

			document.getElementById("status").innerHTML = "SAVED";

        	}
        
        });	

}

function download_configfile() {

	save_configfile();
	
	window.open('/isoident.xml');

};

function clear_configfile() {
	var clearxml = '<isoident><signallib/><messagelib/><devicelib/><evaluation><level1/><level2/><logic/></evaluation></isoident>';
	Xonomy.render(clearxml, editor, docSpec);

	document.getElementById("info").innerHTML = "Konfiguration wurde geaendert.";

	document.getElementById("status").style = "color:#FF0000";
	
	document.getElementById("status").innerHTML = "NOT SAVED";
}