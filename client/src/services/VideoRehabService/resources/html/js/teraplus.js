var videoSources = [];
var currentVideoSourceIndex = 0;

var timerHandle = 0;

var localContact = {'name':'Unknown','uuid':'00000000-0000-0000-0000-000000000000'};

var debounceWheel = 0;
var extraParams;

function connect() {
    initLocalVideo('#selfVideo');
	
 }

function setupSharedObjectCallbacks(channel) {

    //console.error("setupSharedObjectCallbacks " +  channel);

    // Connect to a signal
    channel.objects.SharedObject.newContactInformation.connect(updateContact);
	channel.objects.SharedObject.newVideoSource.connect(selectVideoSource);
	channel.objects.SharedObject.setLocalMirrorSignal.connect(setLocalMirror);
	
    // Check if we need to apply specific parameters
    channel.objects.SharedObject.newExtraParams.connect(setExtraParams);
    channel.objects.SharedObject.getExtraParams();

    // Request contact info...
    channel.objects.SharedObject.getContactInformation();

    // Request current camera
    channel.objects.SharedObject.getCurrentVideoSource();

    // Check if we need to mirror or not the local camera
    channel.objects.SharedObject.getLocalMirror();
}

function updateContact(contact)
{
    //Contact should be a JSON object
	//console.error("Update contact : " + contact);
	
    localContact = JSON.parse(contact);
	
}

function setExtraParams(params){
    if (params === "")
            return;
    console.log("Extra params = " + params);
    extraParams = JSON.parse(params.replace(/'/g, "\""));
}

function setLocalMirror(mirror)
{
	if (mirror){
		document.getElementById("selfVideo").className = "easyrtcMirror";
	}else{
		document.getElementById("selfVideo").className = "";
	}
	
}

function fillVideoSourceList(){
	videoSources.length=0;
	var select = document.getElementById('videoSelect');
	select.options.length = 0;
	var count = 0;

	navigator.mediaDevices.enumerateDevices()
	.then(function(devices) {
		devices.forEach(function(device) {
            if (device.kind === "videoinput"){
				videoSources[videoSources.length] = device;
				//select.options[select.options.length] = new Option(device.label.substring(0,device.label.length-12), device.id);
				select.options[select.options.length] = new Option(device.label, device.id);
				count++;
				if (count<2){
					hideElement("videoSelect"); // Hide if only one video source
				}else{
					showElement("videoSelect");
				}
			}
			select.selectedIndex = currentVideoSourceIndex;
			//console.log(device.kind + ": " + device.label + " id = " + device.deviceId);
		});
	})
	.catch(function(err) {
		console.log(err.name + ": " + err.message);
	});
	
	
	
	
	
}

function updateVideoSource(){
	var select = document.getElementById('videoSelect');
	if (select.selectedIndex>=0){
		currentVideoSourceIndex = select.selectedIndex;
		var constraints = { deviceId: { exact: videoSources[currentVideoSourceIndex].deviceId } };

        // Adjust parameters, if needed
        if (extraParams){
            if (extraParams.cam_width){
                constraints.width = {exact: extraParams.cam_width};
            }
            if (extraParams.cam_height){
                constraints.height = {exact: extraParams.cam_height};
            }
        }
		//console.log(constraints);
		navigator.mediaDevices.getUserMedia({video: constraints}).then(handleVideo).catch(videoError);
		
		SharedObject.cameraChanged(select[select.selectedIndex].label, select.selectedIndex);
		
	}
}

function selectVideoSource(source){
	video = JSON.parse(source);
	for (var i=0; i<videoSources.length; i++){
		if (videoSources[i].label.includes(video.name)){
			var select = document.getElementById('videoSelect');
			select.selectedIndex = i;
			SharedObject.cameraChanged(select[select.selectedIndex].label, select.selectedIndex);
			updateVideoSource();
			break;
		}
	}
	
}

var video = document.getElementById("selfVideo");
 
function initLocalVideo(tag){
	video = document.querySelector(tag);
				 
	navigator.getUserMedia = navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mediaDevices.getUserMedia || navigator.msGetUserMedia || navigator.oGetUserMedia;

	if (navigator.getUserMedia) {       
		//navigator.getUserMedia({video: true, audio: false}, handleVideo, videoError);
		navigator.mediaDevices.getUserMedia({video: true, audio: false}).then(handleVideo).catch(videoError);
	}
}

function handleVideo(stream) {
	var video = document.getElementById("selfVideo");

	//console.log("Success! Device Name: " + stream.getVideoTracks()[0].label);
	video.srcObject = stream;
	//video.src = URL.createObjectURL(stream);
	
	fillVideoSourceList();
}
 
function videoError(e) {
	// do something
	console.log(e);
}


function openButtons(id) {
	document.getElementById(id).style.height = "100%";
}

function closeButtons(id) {
	document.getElementById(id).style.height = "0%";
	stopInactiveTimer();
}

function hideElement(id){
	document.getElementById(id).style.display = "none";
}

function showElement(id){
	document.getElementById(id).style.display = "inline";
}

function toggleButtons(id) {
	if (isButtonsClosed(id))
		openButtons(id);
	else
		closeButtons(id);
	
}

function isButtonsClosed(id){
	return document.getElementById(id).style.height == "0%";
}


function setTitle(id, title){
	var label = "videoLabel";
	if (id!=0){
		label = label + id;
	}
	if (document.getElementById(label)){
		document.getElementById(label).innerText = title;
		titles[id] = title;
	}
}


function resetInactiveTimer(){

	stopInactiveTimer();
	
	timerHandle = setTimeout(inactiveTimeout, 3000);
}

function stopInactiveTimer(){
	if (timerHandle != 0){
		clearTimeout(timerHandle);
		timerHandle = 0;
	}
}

function inactiveTimeout(){
	closeButtons('navButtons');
}

function setPTZCapabilities(uuid, zoom, presets, settings){
	var zoom_tag = "zoomButtons0";
	var presets_tag = "presetButtons0";
	var settings_tag = "settingsButton0";
	
	// Update display
	if (zoom)
		showElement(zoom_tag);
	else
		hideElement(zoom_tag);
		
	if (presets)
		showElement(presets_tag);
	else
		hideElement(presets_tag);
	
	if (settings)
		showElement(settings_tag);
	else
		hideElement(settings_tag);
	

}

function displaySensorMute(uuid, show){
}

function getVideoCoords(event, videoId){
	var video = document.getElementById(videoId);
	
	var video_dims = calculateContainsWindow(video);
	var real_video_width = (video.clientWidth * video_dims.destinationWidthPercentage);
	var bar_width = (video.clientWidth - real_video_width) / 2;
	
	var real_video_height = (video.clientHeight * video_dims.destinationHeightPercentage);
	var bar_height = (video.clientHeight - real_video_height) / 2;
	
	//SharedObject.imageClicked(localContact.uuid, video.clientWidth - (event.clientX - video.offsetLeft), event.clientY - video.offsetTop, video.clientWidth, video.clientHeight);
	SharedObject.imageClicked(localContact.uuid, event.clientX - bar_width, event.clientY - bar_height, real_video_width, real_video_height);
}

function calculateContainsWindow(image) {
	
	var imageComputedStyle = window.getComputedStyle(image);
	var imageObjectFit = imageComputedStyle.getPropertyValue("object-fit");
	var coordinates = {};
	var imagePositions = imageComputedStyle.getPropertyValue("object-position").split(" ");
	var naturalWidth = image.naturalWidth;
	var naturalHeight= image.naturalHeight;
	if( image.tagName === "VIDEO" ) {
		naturalWidth= image.videoWidth;
		naturalHeight= image.videoHeight;
	}
	var horizontalPercentage = parseInt(imagePositions[0]) / 100;
	var verticalPercentage = parseInt(imagePositions[1]) / 100;
	var naturalRatio = naturalWidth / naturalHeight;
	var visibleRatio = image.clientWidth / image.clientHeight;
	if (imageObjectFit === "none")
	{
	  coordinates.sourceWidth = image.clientWidth;
	  coordinates.sourceHeight = image.clientHeight;
	  coordinates.sourceX = (naturalWidth - image.clientWidth) * horizontalPercentage;
	  coordinates.sourceY = (naturalHeight - image.clientHeight) * verticalPercentage;
	  coordinates.destinationWidthPercentage = 1;
	  coordinates.destinationHeightPercentage = 1;
	  coordinates.destinationXPercentage = 0;
	  coordinates.destinationYPercentage = 0;
	}
	else if (imageObjectFit === "contain" || imageObjectFit === "scale-down")
	{
	  // TODO: handle the "scale-down" appropriately, once its meaning will be clear
	  coordinates.sourceWidth = naturalWidth;
	  coordinates.sourceHeight = naturalHeight;
	  coordinates.sourceX = 0;
	  coordinates.sourceY = 0;
	  if (naturalRatio > visibleRatio)
	  {
		coordinates.destinationWidthPercentage = 1;
		coordinates.destinationHeightPercentage = (naturalHeight / image.clientHeight) / (naturalWidth / image.clientWidth);
		coordinates.destinationXPercentage = 0;
		coordinates.destinationYPercentage = (1 - coordinates.destinationHeightPercentage) * verticalPercentage;
	  }
	  else
	  {
		coordinates.destinationWidthPercentage = (naturalWidth / image.clientWidth) / (naturalHeight / image.clientHeight);
		coordinates.destinationHeightPercentage = 1;
		coordinates.destinationXPercentage = (1 - coordinates.destinationWidthPercentage) * horizontalPercentage;
		coordinates.destinationYPercentage = 0;
	  }
	}
	else if (imageObjectFit === "cover")
	{
	  if (naturalRatio > visibleRatio)
	  {
		coordinates.sourceWidth = naturalHeight * visibleRatio;
		coordinates.sourceHeight = naturalHeight;
		coordinates.sourceX = (naturalWidth - coordinates.sourceWidth) * horizontalPercentage;
		coordinates.sourceY = 0;
	  }
	  else
	  {
		coordinates.sourceWidth = naturalWidth;
		coordinates.sourceHeight = naturalWidth / visibleRatio;
		coordinates.sourceX = 0;
		coordinates.sourceY = (naturalHeight - coordinates.sourceHeight) * verticalPercentage;
	  }
	  coordinates.destinationWidthPercentage = 1;
	  coordinates.destinationHeightPercentage = 1;
	  coordinates.destinationXPercentage = 0;
	  coordinates.destinationYPercentage = 0;
	}
	else
	{
	  if (imageObjectFit !== "fill")
	  {
		console.error("unexpected 'object-fit' attribute with value '" + imageObjectFit + "' relative to");
	  }
	  coordinates.sourceWidth = naturalWidth;
	  coordinates.sourceHeight = naturalHeight;
	  coordinates.sourceX = 0;
	  coordinates.sourceY = 0;
	  coordinates.destinationWidthPercentage = 1;
	  coordinates.destinationHeightPercentage = 1;
	  coordinates.destinationXPercentage = 0;
	  coordinates.destinationYPercentage = 0;
	}
	return coordinates;
}

function manageMouseWheel(event){
	// Ignore events for 500 ms
	var currentTime = Date.now();

	if (currentTime > debounceWheel){
		if (event.deltaY > 0){
			zoomOut();
		}else{
			zoomIn();
		}
		debounceWheel = currentTime + 500;
	}
}

function zoomIn(){
	SharedObject.zoomInClicked(localContact.uuid);
	resetInactiveTimer();
}

function zoomOut(){
	SharedObject.zoomOutClicked(localContact.uuid);
	resetInactiveTimer();
}

function zoomMin(){
	SharedObject.zoomMinClicked(localContact.uuid);
}

function zoomMax(){
	SharedObject.zoomMaxClicked(localContact.uuid);
}

function gotoPreset(event, preset){
	if (event.shiftKey && event.ctrlKey)
		SharedObject.setPresetClicked(localContact.uuid, preset);
	else	
		SharedObject.gotoPresetClicked(localContact.uuid, preset);
}

function camSettings(){
	SharedObject.camSettingsClicked(localContact.uuid);
}
