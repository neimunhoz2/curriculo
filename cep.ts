import { Component } from "@angular/core";
import { RouterExtensions } from "nativescript-angular/router";
import { topmost, NavigationEntry, ViewBase } from "tns-core-modules/ui/frame";
import { DbService } from "../shared/db/db.service";
import { ActivatedRoute } from "@angular/router";
import { fromObject } from "data/observable";

@Component({
  selector: "ns-cep",
  moduleId: module.id,
  templateUrl: "./cep.html",
})
export class CepComponent {
  cep: any;

  params = fromObject({
    itemid: 0,
    idcategoria: "",
    idadmin: ""
  });

  constructor(
    private routerExtensions: RouterExtensions,
    private route: ActivatedRoute,
    private db: DbService) {
    this.params.set("itemid", this.route.snapshot.params["itemid"]);
    this.params.set("idcategoria", this.route.snapshot.params["idcategoria"]);
    this.params.set("idadmin", this.route.snapshot.params["idadmin"]);
  }

  cepIsOk(cep): any {
    return this.db
      .geturl("https://viacep.com.br/ws/" + cep + "/json/","application/json");
  }

  continuar() {
    this.cepIsOk(this.cep)
      .subscribe(res => {
        console.dir(<any>res);
        if ((<any>res).erro != true) {
          console.log("cep valido!");
          this.routerExtensions.navigate(
            ["/locais/" +
              this.params.get("itemid") + "/" +
              "inserir/" +
              (<any>res).cep + "/" +
              (<any>res).logradouro + "/" +
              (<any>res).bairro + "/" +
              (<any>res).localidade + "/" +
              (<any>res).uf + "/" +
              this.params.get("idcategoria") + "/" +
              this.params.get("idadmin")],
            {
              clearHistory: false,
             /* transition: {
                name: "flip",
                duration: 1000,
                curve: "linear"
              }*/
            });
        }
      }, (error) => {
        this.onGetDataError(error);
      });
  }

  private onGetDataError(error: Response | any) {
    const body = error.json() || "";
    const err = body.error || JSON.stringify(body);
    console.log("onGetDataError: " + err);
  }

  buscacep() {
    this.routerExtensions.navigate(["/buscacep/" + this.params.get("itemid") + "/" + this.params.get("idcategoria") + "/" + this.params.get("idadmin")], { clearHistory: false });
  }

}
