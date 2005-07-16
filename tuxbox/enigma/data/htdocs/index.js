function setVol(volume)
{
	document.location = "/setVolume?volume=" + volume;
	if (window.screen.width < 800)
		setTimeout("document.location.reload()", 1000);
	else
	{
		headerUpdateVolumeBar(volume, 0);
		data.location.reload();
	}
}

function toggleMute(xy)
{
	document.location = "/setVolume?mute="+xy;
	if (window.screen.width < 800)
		setTimeout("document.location.reload()", 1000);
	else
	{
		headerUpdateVolumeBar(data.volume, 0);
		data.location.reload();
	}
}

function switchChannel(xy, bouquet, channel)
{
	if (window.screen.width < 800)
	{
		NewWindow('/cgi-bin/zapTo?path='+xy+'&curBouquet='+bouquet+'&curChannel='+channel, 'zap', '1', '1', 'no');
	}
	else
	{
		if (zapMode < 4)
			document.location = "/cgi-bin/zapTo?path="+xy+"&curBouquet="+bouquet+"&curChannel="+channel;
		else
			document.location = "?path="+xy+"&mode=zap&zapmode=4&zapsubmode=1";

		setTimeout("parent.data.location.reload()", 500);
		vlcStop();
		setTimeout("vlcStart()", 1000);
	}
}

function logging()
{
	parent.body.document.location = "/log/debug.html";
}

function remoteControl(box)
{
	if (box == "dbox2")
		NewWindow("/showRemoteControl", "RC", "165", "500", "no");
	else
		NewWindow("/showRemoteControl", "RC", "1024", "640", "no");
}

function satFinder(transponder)
{
	NewWindow("/satFinder?" + transponder, "satfind", "170", "150", "no");
}

function deleteMovie(channel)
{
	if (window.confirm('Do you really want to delete this movie?'))
	{
		if (window.screen.width < 800)
		{
			document.location = "/cgi-bin/deleteMovie?ref=" + channel;
		}
		else
		{
			var selChannel = document.channelselector.channel.selectedIndex;
			if (selChannel >= 0)
			{
				currentChannel = selChannel;
				channel = document.channelselector.channel.options[selChannel].value;
				document.location = "/cgi-bin/deleteMovie?ref=" + channel;
				setTimeout("document.location.reload()", 3000);
			}
			else
				alert("Please select a movie first!");
		}
	}
}

function downloadMovie()
{
	if (window.screen.width >= 800)
	{
		var selChannel = document.channelselector.channel.selectedIndex;
		if (selChannel >= 0)
		{
			currentChannel = selChannel;
			var channel = document.channelselector.channel.options[selChannel].value;
			var pos = channel.indexOf("%2f");
			if (pos >= 0)
			{
				var fileName = channel.substr(pos + 3, channel.length - pos - 3);
				fileName = unescape(fileName);
				document.location = "/rootX/" + fileName;
			}
			else 
				alert("Sorry, no valid filename");
		}
		else
			alert("Please select a movie first!");
	}
	else
		alert("Movie download is not supported on this device.");
}

function recoverMovies()
{
	if (window.confirm('Do you really want to rebuild the movie playlist?'))
	{
		NewWindow("/cgi-bin/recoverRecordings", "recoverRecordings", "200", "100", "no", "5000");
	}
}

function openEPG(ref)
{
	NewWindow('/getcurrentepg?type=extended&ref=' + ref, 'EPG', '780', screen.height, 'yes');
}

function openMultiEPG(ref)
{
	NewWindow('/getMultiEPG?ref=' + ref, 'MultiEPG', screen.width, screen.height, 'yes');
}

function admin(command)
{
	NewWindow('/cgi-bin/admin?command=' + command + '&requester=webif', 'admin', '200', '100', 'no', '3000');
}

function openSI()
{
	NewWindow("/xml/streaminfo", "si", "780", "700", "yes");
}

function openChannelInfo()
{
	NewWindow("/cgi-bin/channelinfo", "ci", "780", "600", "yes");
}

function DVRrecord(command)
{
	document.location = "/cgi-bin/videocontrol?command=" + command; 
	setTimeout("document.location.reload()", 500);
}

function sendMessage2TV()
{
	NewWindow("/tvMessageWindow", "msg", "780", "150", "no");
}

function selectAudio()
{
	NewWindow("/cgi-bin/selectAudio?requester=webif", "audio", "250", "130", "no");
}

function selectSubChannel()
{
	NewWindow("/cgi-bin/selectSubChannel", "subchannel", "250", "130", "no");
}

function configSwapFile()
{
	var activate = "off";
	if (swapfileconfig.swap.checked)
		activate = "on";
	document.location = "/cgi-bin/setConfigSwapFile?swap=" + activate + "&swapfile=" + swapfileconfig.swapfile.value;
	setTimeout("document.location.reload()", 500);
}

function vlc()
{
 	document.location = "/video.pls";
 	setTimeout("setStreamingServiceRef()", 200);	
}

function vlcStop()
{
	vlccmd.location = "http://127.0.0.1:8000/?control=stop";
	vlccmd.location = "http://127.0.0.1:8080/?control=empty";
}

function vlcStart()
{
	if (parent.data.vlcparms)
	{
		if (parent.data.vlcparms.indexOf("ffffffff") == -1)
		{
			vlccmd.location.href = "http://127.0.0.1:8080/?control=add&mrl=" + parent.data.vlcparms;
			setTimeout("vlcStartItem()", 200);
		}
		else
		{
			parent.data.location.reload();
			setTimeout("vlcStart()", 500);
		}
	}
	else
		setTimeout("vlcStart()", 200);
}

function vlcStartItem()
{
	vlccmd.location.href = "http://127.0.0.1:8080/?control=play&item=0";
	setTimeout("setStreamingServiceRef()", 200);
}

function setStreamingServiceRef()
{
	if (parent.data.serviceReference)
		document.location = "/cgi-bin/setStreamingServiceRef?sref=" + parent.data.serviceReference;
	else
		setTimeout("setStreamingServiceRef()", 200);
}
