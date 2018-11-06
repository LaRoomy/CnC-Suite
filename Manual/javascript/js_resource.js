function autoResizeElement(Id) {

 	var element = document.getElementById(Id);
 	var doc = element.contentDocument ? element.contentDocument : element.contentWindow.document;
 	var body_ = doc.body, html_ = doc.documentElement;
 	var height = Math.max( body_.scrollHeight, body_.offsetHeight, html_.clientHeight, html_.scrollHeight, html_.offsetHeight );

	element.style.visibility = 'hidden';
	element.style.height = "10px";
	element.style.height = height + 5+"px";
	element.style.visibility = 'visible';
 }

 function setPage(loc, buttonName) {

 	var iframe = document.getElementById('contentFrame')

 	// if this is chrome change the attributes, because the sizer will not work properly
	var isChrome = !!window.chrome && !!window.chrome.webstore;
 	if(isChrome)
 	{
 		iframe.setAttribute('scrolling', 'yes');
 		iframe.style.height = '80%';
 	}

 	// set new source
    iframe.setAttribute('src', loc);

    // set the color of the selected button
    // and restore the defaultcolor of the other buttons
    var buttons = document.getElementsByTagName('button');

    for(var i=0;i<buttons.length;i++){

    	if(buttons[i].name == buttonName)
    		buttons[i].style.background='#FFCC00';
    	else
    		buttons[i].style.background='orange';
    }
 }