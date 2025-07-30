import { Component, OnInit } from "@angular/core";
import { RouterExtensions } from "nativescript-angular/router";
import { DbService } from "../shared/db/db.service";
import { ListPicker } from "tns-core-modules/ui/list-picker";
import { StackLayout } from "ui/layouts/stack-layout";
import { Page } from "tns-core-modules/ui/page";
import { Color } from "color";
import { fromObject } from "data/observable";
import { TextField } from "ui/text-field";
import { ActivatedRoute } from "@angular/router";
import { TimePicker } from "ui/time-picker";
import { DatePicker } from "ui/date-picker";

import { SearchBar } from "ui/search-bar";
import { ItemService } from "./item.service";
import { Item } from "./item";


@Component({
  moduleId: module.id,
  templateUrl: "./eventos.html",
})
export class EventosComponent implements OnInit {
  public artistas: any[];
  titulo: string = "ARTISTA"
  public estilos: any[];
  public locais: any[];
  pagenumber: number = 0;
  curartista: any;
  curestilo: any;
  curlocal: any;
  isLoading: boolean = true;
  evento = fromObject({
    titulo: "",
    descricao: ""
  });

  params = fromObject({
    itemid: 0,
    idcategoria: "",
    idadmin: ""
  });
  
  item: Item;

  constructor(
    private itemService: ItemService,
    private routerExtensions: RouterExtensions,
    private db: DbService,
    private route: ActivatedRoute,
    private page: Page) {
    this.artistas = [];
    this.estilos = [];
    this.locais = [];
    
    
  }

  ngOnInit() {
    this.params.set("acao", this.route.snapshot.params["acao"]);
    var itemid=this.route.snapshot.params["itemid"];
    this.params.set("itemid", itemid);
    this.item = this.itemService.getItem(itemid); 
    this.params.set("idcategoria", this.route.snapshot.params["idcategoria"]);
    this.params.set("idadmin", this.route.snapshot.params["idadmin"]);
    this.loadlist(this.artistas, "artistas");

    this.loadlist(this.estilos, "estilos");
    this.loadlist(this.locais, "locais");

  }

  selectedIndexChanged(arg) {
    switch ((<any>arg.object).id) {
      case "artistas":
        this.curartista = (<any>arg.object).items[(<any>arg.object).selectedIndex];
        console.dir(this.curartista);
        break;
      case "estilos":
        this.curestilo = (<any>arg.object).items[(<any>arg.object).selectedIndex];


        console.dir(this.curestilo);
        break;
      case "locais":
        this.curlocal = (<any>arg.object).items[(<any>arg.object).selectedIndex];

        console.dir(this.curlocal);
        break;
    }

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
    var lstpick: ListPicker = <ListPicker>this.page.getViewById(lstpickid);

    lstpick.items = items;
  }

  searchPhrase = "";
  Searchhint = "Digite o artista";

  public onTextChanged(args) {
    let searchBar = <SearchBar>args.object;
    this.filterlistpicker(this.artistas, "artistas", searchBar.text);
    console.log("SearchBar text changed! New value: " + searchBar.text);

  }

  loadlist(array, key) {
    this.db
      .get("key=" + key + "&idcategoria=" + this.params.get("idcategoria") + "&idadmin=" + this.params.get("idadmin"))
      .subscribe(res => {
        if (res != null) {
          (<any>res).result.forEach(row => {
            array.push({
              row,
              toString: () => { return row.nome; },
            })
          });
          var pickUF: ListPicker = <ListPicker>this.page.getViewById(key);
          pickUF.items = array;
          pickUF.selectedIndex = 0;
          switch (key) {
            case "artistas":
              this.curartista = pickUF.items[pickUF.selectedIndex];


              console.dir(this.curartista);
              break;
            case "estilos":
              this.curestilo = pickUF.items[pickUF.selectedIndex];
              console.dir(this.curestilo);
              break;
            case "locais":
              this.curlocal = pickUF.items[pickUF.selectedIndex];
              console.dir(this.curlocal);
              break;
          }

          console.dir(array);
        }
        this.isLoading = false;
      });
  }

  onclick() {
    this.pagenumber++;
    switch (this.pagenumber) {
      case 1:
        this.Searchhint = "Digite o estilo";
        this.titulo = "ESTILOS"
        break;
      case 2:
        this.Searchhint = "Digite o local";
        this.titulo = "LOCAIS"
        break;
      case 5:
        this.titulo = "DADOS DO EVENTO";
        var txt: TextField = <TextField>this.page.getViewById("titulo");
        setTimeout(() => {
          txt.focus();
        }, 100);
        break;
      case 3:
        this.titulo = "DATA"
        break;
      case 4:
        this.titulo = "HORÁRIO"
        break;
    }
    this.searchPhrase = "";
  }

  save() {
    this.db
      .put({
        op: 'adicionar',
        key: 'eventos',
        idadmin: this.params.get("idadmin"),
        idestilo: this.curestilo.row.id,
        idartista: this.curartista.row.id,
        idcategoria: this.params.get("idcategoria"),
        idlocal: this.curlocal.row.id,
        datahorario: this.date + " " + this.time,
        nome: this.evento.get("titulo"),
        descricao: this.evento.get("descricao"),
      })
      .subscribe(res => {
       this.routerExtensions.backToPreviousPage();
        this.item.menu.push({
         key: (<any>res).key, name: (<any>res).result.nome, id: (<any>res).result.id, menu: null,
        });
        this.item.menu.sort(function (a, b) {
         var nameA = a.name.toLowerCase(), nameB = b.name.toLowerCase();
         if (nameA < nameB)
          return -1;
         if (nameA > nameB)
          return 1;
         return 0;
        });
        console.dir(res);
        console.log((<any>res).status);
      });

  }

  onPickerLoaded(args) {
    let datePicker = <DatePicker>args.object;    
    datePicker.minDate = new Date(Date.now());
    datePicker.maxDate = new Date(2045, 4, 12);
    this.date=datePicker.date.toISOString().slice(0, 10);
  }

  onDateChanged(args) {
    let datePicker = <DatePicker>args.object;
    this.date=datePicker.date.toISOString().slice(0, 10);
    console.log(this.date);
  }

  
  onDTPickerLoaded(args) {
    let timePicker = <TimePicker>args.object;
    timePicker.hour = 20;
    timePicker.minute = 0;
  }

  time:string="20:00:00";
  date:String="";

  onTimeChanged(args) {
    let timePicker = <TimePicker>args.object;
    this.time=timePicker.time.toTimeString().slice(0,8);
    console.log(this.time);
  }

}
