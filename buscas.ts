import { Component, OnInit, ViewChild, ElementRef } from "@angular/core";
import { RouterExtensions } from "nativescript-angular/router";
import { DbService } from "../shared/db/db.service";
import { ListPicker } from "tns-core-modules/ui/list-picker";
import { Page } from "tns-core-modules/ui/page";
import { fromObject } from "tns-core-modules/data/observable";
import { ActivatedRoute } from "@angular/router";
import { TimePicker } from "tns-core-modules/ui/time-picker";
import { DatePicker } from "tns-core-modules/ui/date-picker";
import { SearchBar } from "tns-core-modules/ui/search-bar";
import { ItemService } from "./item.service";
import { Item } from "./item";
import { WebView, LoadEventData } from "tns-core-modules/ui/web-view";
import { MapView, Marker, Position, Bounds } from 'nativescript-google-maps-sdk';
import { registerElement } from 'nativescript-angular/element-registry';
import { clearWatch, watchLocation, Location, getCurrentLocation, isEnabled, distance, enableLocationRequest } from "nativescript-geolocation";
import { Accuracy } from "ui/enums";

registerElement('MapView', () => MapView);

@Component({
  moduleId: module.id,
  templateUrl: "./buscas.html",
})
export class BuscasComponent implements OnInit {
  titulo: string = "";
  searchPhrase: string = "";
  Searchhint: string = "";
  public cidades: any[] = [];
  public locais: any[] = [];
  public estados: any[] = [];
  public bairros: any[] = [];
  public eventos: any[] = [];
  public estilos: any[] = [];
  public artistas: any[] = [];
  pagenumber: number = 0;
  curartista: any;
  curcidade: any;
  curlocal: any;
  curestado: any;
  curbairro: any;
  curevento: any;
  curestilo: any;
  isLoading: boolean = true;
  showwebview: boolean = false;
  showmap: boolean = false;
  tipobusca: string = "";

  params = fromObject({
    itemid: 0,
    idcategoria: "",
    idadmin: ""
  });

  item: Item;
  @ViewChild("myWebView") webViewRef: ElementRef;

  constructor(
    private itemService: ItemService,
    private routerExtensions: RouterExtensions,
    private db: DbService,
    private route: ActivatedRoute,
    private page: Page,
  ) {
    this.location.latitude = 0;
    this.location.longitude = 0;
  }

  /*isLocationEnabled() {
    // >> check-is-service-enabled
    isEnabled().then(function (isLocationEnabled) {
        let message = "Location services are not available";
        if (isLocationEnabled) {
            message = "Location services are available";
        }
        alert(message);
    }, function (e) {
        console.log("Location error received: " + (e.message || e));
    });
    // << check-is-service-enabled
}*/

  public startpointLongitude: number = 42.696552;
  public startpointLatitude: number = 23.32601;
  /*
  
  */

  ngOnInit() {
    //this.isLocationEnabled();
    //this.getLocationOnce();
    isEnabled().then(function (isEnabled) {
      if (!isEnabled) {
        enableLocationRequest().then(function () {
          alert("localizacão habilitada");
        }, function (e) {
          console.log("Erro: " + (e.message || e)); alert("localizacão indisponível");
        });
      }
    }, function (e) {
      console.log("Erro: " + (e.message || e)); alert("localizacão indisponível");
    });
    let webview: WebView = this.webViewRef.nativeElement;
    webview.on(WebView.loadFinishedEvent, function (args: LoadEventData) {
      let message;
      if (!args.error) {
        message = "WebView finished loading of " + args.url;
      } else {
        message = "Error loading " + args.url + ": " + args.error;
      }

      //label.text = message;
      console.log("WebView message - " + message);
    });
    console.dir(this.route.snapshot.params);
    this.tipobusca = this.route.snapshot.params["key"];
    this.params.set("key", this.route.snapshot.params["key"]);
    var itemid = this.route.snapshot.params["itemid"];
    this.params.set("itemid", itemid);
    this.item = this.itemService.getItem(itemid);
    this.params.set("idcategoria", this.route.snapshot.params["idcategoria"]);
    this.params.set("idadmin", this.route.snapshot.params["idadmin"]);
    this.atualizapageparams(false);
  }

  updateLstPickCurrent(lstpck) {
    switch (lstpck.id) {
      case "estilosevt":
        this.curestilo = lstpck.items[lstpck.selectedIndex];
        console.dir(this.curestilo);
        break;
      case "estadosevt":
        this.curestado = lstpck.items[lstpck.selectedIndex];
        console.dir(this.curestado);
        break;
      case "artistasevt":
        this.curartista = lstpck.items[lstpck.selectedIndex];
        console.dir(this.curartista);
        break;
      case "cidadesevt":
        this.curcidade = lstpck.items[lstpck.selectedIndex];
        console.dir(this.curcidade);
        break;
      case "locais":
        this.curlocal = lstpck.items[lstpck.selectedIndex];
        console.dir(this.curlocal);
        break;
      case "bairrosevt":
        this.curbairro = lstpck.items[lstpck.selectedIndex];
        console.dir(this.curbairro);
        break;
    }
  }

  selectedIndexChanged(arg) {
    this.updateLstPickCurrent(<any>arg.object);
  }

  public onSubmit(args) {

  }

  filterlistpicker(array, lstpickid, text) {
    var items = [];
    array.forEach(row => {
      var n = row.row.nome.toLowerCase().search(text.toLowerCase());
      if (n === 0)
        items.push(row);
    });
    var lstpick: any = <any>this.page.getViewById(lstpickid);

    lstpick.items = items;
    this.updateLstPickCurrent(lstpick);
  }

  public onTextChanged(args) {
    let searchBar = <SearchBar>args.object;
    console.log("pgno=" + this.pagenumber)
    switch (this.pagenumber) {
      case 0:
        this.filterlistpicker(this.estilos, "estilosevt", searchBar.text);
        break;
      case 1:
        this.filterlistpicker(this.estados, "estadosevt", searchBar.text);
        break;
      case 2:
        this.filterlistpicker(this.cidades, "cidadesevt", searchBar.text);
        console.dir(this.curcidade);
        break;
      case 3:
        this.filterlistpicker(this.bairros, "bairrosevt", searchBar.text);
        break;
      case 4:
        this.filterlistpicker(this.eventos, "eventosregiao", searchBar.text);
        break;
    }

    console.log("SearchBar text changed! New value: " + searchBar.text);

  }

  buscaevt() {

  }

  loadlist(array, key) {

    this.isLoading = true;
    this.db
      .get("key=" + key +
        "&idcategoria=" + this.params.get("idcategoria") +
        "&idadmin=" + this.params.get("idadmin") +
        "&uf=" + (this.curestado == undefined ? "" : this.curestado.row.uf) +
        "&cidade=" + (this.curcidade == undefined ? "" : this.curcidade.row.id) +
        "&bairro=" + (this.curbairro == undefined ? "" : this.curbairro.row.nome) +
        "&idestilo=" + (this.curestilo == undefined ? "" : this.curestilo.row.id) +
        "&idartista=" + (this.curartista == undefined ? "" : this.curartista.row.id))
      .subscribe(res => {
        if (res != null) {
          //array = [];
          (<any>res).result.forEach(row => {
            array.push({
              row,
              toString: () => { return row.nome; },
            })
          });
          var pickUF: any = <any>this.page.getViewById(key);
          pickUF.items = array;
          pickUF.selectedIndex = 0;
          this.updateLstPickCurrent(pickUF);
          // console.dir(array);
        }
        this.isLoading = false;
      });
  }

  atualizapageparams(isgoback: Boolean) {
    this.searchPhrase = "";
    switch (this.pagenumber) {
      case 0:
        this.Searchhint = "Digite o estilo";
        this.titulo = "ESTILO";
        if (!isgoback)
          this.loadlist(this.estilos, "estilosevt");
        break;
      case 1:
        if (this.tipobusca == "artistas") {
          this.Searchhint = "Digite o artista";
          this.titulo = "ARTISTA";
          if (!isgoback){this.artistas=[];
            this.loadlist(this.artistas, "artistasevt");
          }

        }
        else {
          this.Searchhint = "Digite o estado";
          this.titulo = "ESTADO";
          if (!isgoback)
            this.loadlist(this.estados, "estadosevt");

        }

        break;
      case 2:
        this.Searchhint = "Digite a cidade";
        this.titulo = "CIDADE";
        if (!isgoback)
          this.loadlist(this.cidades, "cidadesevt");
        break;
      case 3:
        this.Searchhint = "Digite o bairro";
        this.titulo = "BAIRRO";
        if (!isgoback)
          this.loadlist(this.bairros, "bairrosevt");
        break;
      case 4:
        this.titulo = "EVENTOS";
        this.Searchhint = "Digite o evento";
        if (!isgoback) {
          this.eventos = [];
          this.loadlist(this.eventos, "eventosregiao");
        }
        break;
      case 5:
        this.titulo = "EVENTO"
        break;
    }
  }

  onclick() {
    switch (this.tipobusca) {
      case "regiao":
        this.pagenumber++;
        break;
      case "estilos":
        this.pagenumber = 4;
        break;
      case "artistas":
        if (this.pagenumber == 1)
          this.pagenumber = 4;
        else
          this.pagenumber++;
        break;
    }

    this.atualizapageparams(false);
  }

  goBack() {
    if (this.showwebview) {
      let webview: WebView = this.webViewRef.nativeElement;
      if (webview.canGoBack) {
        webview.goBack();
      }
      webview.src = "";
      this.showwebview = false;
    }
    else if (this.showmap) {
      this.showmap = false;
    }
    else {
      if (this.pagenumber > 0) {
        if (this.tipobusca == "estilos" && this.pagenumber == 4)
          this.pagenumber = 0;
        else if (this.tipobusca == "artistas" && this.pagenumber == 4)
          this.pagenumber = 1;
        else
          this.pagenumber--;
        this.atualizapageparams(true);
      }
      else
        this.routerExtensions.backToPreviousPage();
    }

  }


  abresite() {
    let webview: WebView = this.webViewRef.nativeElement;
    webview.src = this.curevento.row.site;
    this.showwebview = true;
    if (this.watchId) {
      clearWatch(this.watchId);
    }
  }

  eventoclick(item) {
    this.curevento = item;
    var t = this.curevento.row.datahorario.split(/[- :]/);
    var data = new Date(Date.UTC(t[0], t[1] - 1, t[2], t[3], t[4], t[5]));
    var semana = ["Domingo", "Segunda-Feira", "Terça-Feira", "Quarta-Feira", "Quinta-Feira", "Sexta-Feira", "Sábado"];
    var diames = data.getUTCDate() < 10 ? "0" + data.getUTCDate() : data.getUTCDate();
    var mes = data.getUTCMonth() < 9 ? "0" + (data.getUTCMonth() + 1) : (data.getUTCMonth() + 1);
    var hora = data.getUTCHours() < 10 ? "0" + data.getUTCHours() : data.getUTCHours();
    var minutos = data.getUTCMinutes() < 10 ? "0" + data.getUTCMinutes() : data.getUTCMinutes();
    this.curevento.row.data = diames + "\\" + mes + "\\" + data.getUTCFullYear() + " - " + semana[data.getDay()];
    this.curevento.row.time = hora + ":" + minutos;
    this.pagenumber++;
    this.atualizapageparams(false);
    console.dir(this.curevento);

    console.dir(data);

  }

  onPickerLoaded(args) {
    let datePicker = <DatePicker>args.object;
    datePicker.minDate = new Date(Date.now());
    datePicker.maxDate = new Date(2045, 4, 12);
    this.date = datePicker.date.toISOString().slice(0, 10);
  }

  onDateChanged(args) {
    let datePicker = <DatePicker>args.object;
    this.date = datePicker.date.toISOString().slice(0, 10);
    console.log(this.date);
  }


  onDTPickerLoaded(args) {
    let timePicker = <TimePicker>args.object;
    timePicker.hour = 20;
    timePicker.minute = 0;
  }

  time: string = "20:00:00";
  date: String = "";

  onTimeChanged(args) {
    let timePicker = <TimePicker>args.object;
    this.time = timePicker.time.toTimeString().slice(0, 8);
    console.log(this.time);
  }

  printUISettings(settings) {
    console.log("Current UI setting:\n" + JSON.stringify({
      compassEnabled: settings.compassEnabled,
      indoorLevelPickerEnabled: settings.indoorLevelPickerEnabled,
      mapToolbarEnabled: settings.mapToolbarEnabled,
      myLocationButtonEnabled: settings.myLocationButtonEnabled,
      rotateGesturesEnabled: settings.rotateGesturesEnabled,
      scrollGesturesEnabled: settings.scrollGesturesEnabled,
      tiltGesturesEnabled: settings.tiltGesturesEnabled,
      zoomControlsEnabled: settings.zoomControlsEnabled,
      zoomGesturesEnabled: settings.zoomGesturesEnabled
    }, undefined, 2));
  }

  location: Location = new Location();
  zoom = 15;
  minZoom = 0;
  maxZoom = 40;
  bearing = 0;
  tilt = 0;
  padding = [40, 40, 40, 40];
  mapView: MapView;

  lastCamera: String;
  watchId: any;

  getlatlongFromEnd() {
    var url: String = "https://maps.googleapis.com/maps/api/geocode/json?address=";
    var end = this.curevento.row.logradouro.split(" ");
    end.forEach(e => {
      url = url + e + "+";
    });
    url.replace(/.$/, ",");
    url = url + "+" + this.curevento.row.numero + "+-+";
    var bair = this.curevento.row.bairro.split(" ");
    bair.forEach(e => {
      url = url + e + "+";
    });
    url.replace(/.$/, ",");
    url = url + "+";

    var cid = this.curevento.row.localidade.split(" ");
    cid.forEach(e => {
      url = url + e + "+";
    });
    url.replace(/.$/, ",");
    url = url + "-+" + this.curevento.row.uf + ",+" + this.curevento.row.cep;
    url = url + "&key=AIzaSyBjF_58qpK1CsH2SMZdhNtFmab87Q4wfWU";
    return this.db.geturl(url, "application/json");
  }

  getLocationOnce() {
    return getCurrentLocation({
      desiredAccuracy: Accuracy.high,
      updateDistance: 5,
      timeout: 5000
    });
  }

  abremapa() {
    if (this.watchId) {
      clearWatch(this.watchId);
    }
    this.getlatlongFromEnd()
      .subscribe(res => {
        this.db.geturl("https://maps.googleapis.com/maps/api/streetview?size=300x300&location=-25.116047692839533,-50.17011445015669&fov=150&heading=60&pitch=-60&key=AIzaSyCpX-cfRtq9NrAeY1DRUs1uoLxeMwK_a4I", "image/jpeg")
          .subscribe(r => {
            console.log("streetview: ");
            console.log(<any>r.toString);
          });

        this.getLocationOnce()
          .then(location => {
            console.log("Location received: ");
            console.dir(location);
            this.startpointLatitude = location.latitude;
            this.startpointLongitude = location.longitude;
            console.dir(<any>res);
            this.location.latitude = (<any>res).results[0].geometry.location.lat;//location.latitude;// (<any>res).results[0].geometry.location.lat;
            this.location.longitude = (<any>res).results[0].geometry.location.lng; //location.longitude;//(<any>res).results[0].geometry.location.lng;
            this.location.altitude = 0;

            var marker = new Marker();
            marker.position = Position.positionFromLatLng(this.location.latitude, this.location.longitude);
            marker.title = this.curevento.row.nomelocal;
            marker.snippet = this.curevento.row.nome;
            marker.userData = { index: 0 };
            marker.visible = true;
            this.mapView.addMarker(marker);
            marker = new Marker();
            marker.position = Position.positionFromLatLng(this.startpointLatitude, this.startpointLongitude);
            marker.title = "EU";
            marker.snippet = "bora lá";
            marker.userData = { index: 1 };
            marker.visible = true;
            this.mapView.addMarker(marker);

            marker = this.mapView.findMarker(function (marker) {
              return marker.userData.index === 1;
            });
            console.log("Moving marker...", marker.userData);
            var __this = this;

            this.watchId = watchLocation(
              function (loc) {
                if (loc) {
                  marker = __this.mapView.findMarker(function (marker) {
                    return marker.userData.index === 1;
                  });
                  loc.altitude = 0;

                  marker.position = Position.positionFromLatLng(loc.latitude, loc.longitude);
                  marker.snippet = "distância = " + distance(loc, __this.location).toFixed(2) + " metros";
                  // console.log("Received location: " + loc.latitude);
                  // console.log("Received longitude: " + loc.longitude);
                  // console.dir(loc);
                  marker.showInfoWindow();
                }
              },
              function (e) {
                console.log("Error: " + e.message);
              },
              { desiredAccuracy: Accuracy.high, updateDistance: 3, minimumUpdateTime: 1000 * 3 });

            this.mapView.myLocationEnabled = true;
            this.mapView.settings.zoomControlsEnabled = true;
            this.mapView.settings.compassEnabled = true;
            this.mapView.settings.indoorLevelPickerEnabled = true;
            this.mapView.settings.mapToolbarEnabled = true;
            this.mapView.settings.myLocationButtonEnabled = true;
            this.mapView.settings.rotateGesturesEnabled = true;
            this.mapView.settings.scrollGesturesEnabled = true;
            this.mapView.settings.tiltGesturesEnabled = true;
            this.mapView.settings.zoomGesturesEnabled = true;
            //***  setViewport não funciona
            /*var southwest=Position.positionFromLatLng(this.startpointLatitude,this.startpointLongitude),
             northeast=Position.positionFromLatLng(this.latitude,this.longitude);
             let tmplat=southwest.latitude;
             southwest.latitude=Math.min(southwest.latitude,northeast.latitude);
             northeast.latitude=Math.max(tmplat,northeast.latitude);
             tmplat=southwest.longitude;
             southwest.longitude=Math.max(southwest.longitude,northeast.longitude);
             northeast.longitude=Math.min(tmplat,northeast.longitude);

             let viewport = {
              southwest: {
                  latitude: southwest.latitude + 0.001,
                  longitude: northeast.longitude + 0.001
              },
              northeast: {
                  latitude: northeast.latitude - 0.001,
                  longitude: southwest.longitude - 0.001
              }
          }

            var bounds=Bounds.fromCoordinates(
              Position.positionFromLatLng(viewport.southwest.latitude,viewport.southwest.longitude),
              Position.positionFromLatLng(viewport.northeast.latitude,viewport.northeast.longitude)
            );*/

            // this.mapView.setViewport(bounds, 100);
            //console.dir(bounds.southwest.latitude);
            this.showmap = true;
          }).catch(error => {
            console.log("Location error received: " + error);
            alert("Location error received: " + error);
          });

      });

  }

  onMapReady(event) {
    console.log('Map Ready');
    this.mapView = event.object;
  }

  onCoordinateTapped(args) {
    console.log("Coordinate Tapped, Lat: " + args.position.latitude + ", Lon: " + args.position.longitude, args);
  }

  webViewTouch(args) {
    console.log(`Touch: x: ${args.getX().toFixed(3)} y: ${args.getY().toFixed(3)}`);
  }
  // setting up Pan gesture callback method
  webViewPan(args) {
    this.heading = (this.heading + args.deltaX) % 360;

    this.pitch = (this.pitch + args.deltaY) % 90;
    let webview: WebView = this.webViewRef.nativeElement;
    /*webview.src =//"https://maps.googleapis.com/maps/api/streetview?size=300x300&location=-25.116047692839533,-50.17011445015669&fov=150&heading="+this.heading+"&pitch="+this.pitch+"&key=AIzaSyCpX-cfRtq9NrAeY1DRUs1uoLxeMwK_a4I"
      '<head><meta name="viewport" content="width=device-width, minimum-scale=0.1"><title>streetview (300×300)</title></head>\
   <body style="margin: 0px; background: #0e0e0e;"><img style="-webkit-user-select: none;" src="https://maps.googleapis.com/maps/api/streetview?size=300x300&amp;location=-25.116047692839533,-50.17011445015669&amp;fov=150&amp;heading='+ this.heading + '&pitch=' + this.pitch + '&amp;key=AIzaSyCpX-cfRtq9NrAeY1DRUs1uoLxeMwK_a4I"></body>'
*/
    console.log(`Pan: deltaX: ${args.deltaX.toFixed(3)} deltaY: ${args.deltaY.toFixed(3)}`);
  }
  heading: number = 60;
  pitch: number = -30;
  onMarkerEvent(args) {
    console.log("Marker Event: '" + args.eventName
      + "' triggered on: " + args.marker.title
      + ", Lat: " + args.marker.position.latitude + ", Lon: " + args.marker.position.longitude, args);
    let webview: WebView = this.webViewRef.nativeElement;
    // webview.src = this.curevento.row.site;
    webview.src =//"https://maps.googleapis.com/maps/api/streetview?size=300x300&location=-25.116047692839533,-50.17011445015669&fov=150&heading="+this.heading+"&pitch="+this.pitch+"&key=AIzaSyCpX-cfRtq9NrAeY1DRUs1uoLxeMwK_a4I"
      /*  '<head><meta name="viewport" content="width=device-width, minimum-scale=0.1"><title>streetview (300×300)</title></head>\
      <body style="margin: 0px; background: #0e0e0e;"><img style="-webkit-user-select: none;" src="https://maps.googleapis.com/maps/api/streetview?size=300x300&amp;location=-25.116047692839533,-50.17011445015669&amp;fov=150&amp;heading='+ this.heading + '&pitch=' + this.pitch + '&amp;key=AIzaSyCpX-cfRtq9NrAeY1DRUs1uoLxeMwK_a4I"></body>'
      */
      '<!DOCTYPE html>\
    <html>\
      <head>\
        <meta charset="utf-8">\
        <title>Street View side-by-side</title>\
        <style>\
          html, body {\
            height: 100%;\
            margin: 0;\
            padding: 0;\
          }\
          #map {\
            float: left;\
            height: 300px;\
            width: 0%;\
            display:none;\
          }\
          #pano {\
            float: left;\
            height: 300px;\
            width: 300px;\
          }\
        </style>\
      </head>\
      <body>\
        <div id="map"></div>\
        <div id="pano"></div>\
        <script>\
          function initialize() {\
            var fenway = {lat: '+ args.marker.position.latitude + ', lng: ' + args.marker.position.longitude + '};\
            var map = new google.maps.Map(document.getElementById("map"), {\
              center: fenway,\
              zoom: 14\
            });\
            var panorama = new google.maps.StreetViewPanorama(\
                document.getElementById("pano"), {\
                  position: fenway,\
                  pov: {\
                    heading: 34,\
                    pitch: 10\
                  }\
                });\
            map.setStreetView(panorama);\
          }\
        </script>\
        <script async defer\
            src="https://maps.googleapis.com/maps/api/js?key=AIzaSyCpX-cfRtq9NrAeY1DRUs1uoLxeMwK_a4I&callback=initialize">\
        </script>\
      </body>\
    </html>'
    this.showwebview = true;
    this.showmap = false;
    /*let opts: AdvancedWebView.AdvancedWebViewOptions = {
      url: 'https://google-developers.appspot.com/maps/documentation/javascript/examples/full/streetview-simple?hl=pt',
      toolbarColor: '#ff4081',
      toolbarControlsColor: '#333', // iOS only
      showTitle: false // Android only
  };

  AdvancedWebView.openAdvancedUrl(opts);*/
  }

  onCameraChanged(args) {
    console.log("Camera changed: " + JSON.stringify(args.camera), JSON.stringify(args.camera) === this.lastCamera);
    this.lastCamera = JSON.stringify(args.camera);
  }




}
