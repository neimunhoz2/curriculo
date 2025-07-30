#include "mdi.h"
#include "splinha.h"
#include "csg.h"
#include "cmdtrim.h"
#include "renderconc.h"
#include "hatch.h"
#include "bhatch.h"
#include "mtext.h"
#include "dimension.h"
#include "arc.h"
#include "circle.h"
#include "attrib.h"
#include "dxf.h"
#include "ctrl_macros.h"
#include "texturas.h"
#include "texto.h"


Vertice::Vertice(){
 FLAG=0;
}

no::no(void *col,int ptr){
 colna=col;
 endptr=ptr;
 if(((colunaptr*)col)->ant)
 {
  esq=((colunaptr*)col)->ant->atlno;
  dir=0;
  ((colunaptr*)col)->ant->atlno->dir=this;
 }
 else
 {
  esq=0;
  dir=0;
 }
}

colunaptr::colunaptr(){
 prmno=ultno=atlno=0;
}

linhaptr::linhaptr(){
 prmno=ultno=atlno=0;
}

MatrizPtr::MatrizPtr(){
 numcol=numlin=0;
 prmlin=ultlin=atllin=0;
 prmcln=ultcln=atlcln=0;
}

MatrizPtr::~MatrizPtr(){
 limpa();
}

void MatrizPtr::novacoluna(){
 atlcln=new colunaptr();
 if(numcol==0)
  prmcln=atlcln;
 if(ultcln)
  ultcln->prx=atlcln;
 atlcln->ant=ultcln;
 atlcln->prx=0;
 ultcln=atlcln;
 numcol++;
}

void MatrizPtr::limpa(){
 colunaptr *cln,*prxcln;
 no *cel,*prxcel;
 cln=prmcln;
 while(cln){
  cel=cln->prmno;
  while(cel){
   prxcel=cel->prx;
   delete cel;
   cel=prxcel;
  }
  prxcln=cln->prx;
  delete cln;
  cln=prxcln;
 }
 linhaptr *grv=prmlin,*prxgrav;
 while(grv){
  prxgrav=grv->prx;
  delete grv;
  grv=prxgrav;
 }
 numcol=numlin=0;
 prmlin=ultlin=atllin=0;
 prmcln=ultcln=atlcln=0;
}

void MatrizPtr::novalinhaptr(Vetor *v){
 colunaptr *col=prmcln;
 if(!col)
  return; //ShowMessage("nenhuma coluna definida");
 atllin=new linhaptr();
 if(v){
  Vetor_detvalfast(&atllin->v,v);
 }
 if(numlin==0)
  prmlin=atllin;
 if(ultlin)
  ultlin->prx=atllin;
 atllin->ant=ultlin;
 atllin->prx=0;
 ultlin=atllin;
 numlin++;
 while(col){
  col->atlno=new no((void*)col,0);
  if(!col->prmno)
   col->prmno=col->atlno;
  if(!atllin->prmno)
   atllin->prmno=col->atlno;
  if(col->ultno)
   col->ultno->prx=col->atlno;
  col->atlno->ant=col->ultno;
  col->atlno->prx=0;
  col->ultno=col->atlno;
  atllin->ultno=col->atlno;
  atllin->atlno=col->atlno;
  col=col->prx;
 }
}

void MatrizPtr::apagalinhaptr(linhaptr *lin){
 if(numlin==0||!lin)return;
 if(lin->prx)
  lin->prx->ant=lin->ant;
 else
  ultlin=lin->ant;
 if(lin->ant)
  lin->ant->prx=lin->prx;
 else
  prmlin=lin->prx;
 if(lin->ant)
  atllin=lin->ant;
 else
  atllin=lin->prx;
 delete lin;
 numlin--;
}


void ListaVertices::Limpa(){
 vertices.limpa();
 limpa();
}

ListaVertices::~ListaVertices(){
 Limpa();
}

void ListaVertices::novo(NDOUBLE x,NDOUBLE y,NDOUBLE z,unsigned char FLAG){
 novoptr();
 atual->FLAG=FLAG;
 atual->pt.flag=FLAG;
 Vetor_detvalfast2(&atual->pt,x,y,z);
 Vetor_vetornulo(&atual->mednorm);
 atual->ptela.detvalfast(0,0);
}

Vertice *ListaVertices::jaexisteverts(Vetor *p){
 Vertice *vt;
 for(vt=prm;vt;vt=vt->prx){
  if(__pontosiguais(vt->pt,*p))
   return vt;
 }
 return 0;
}

void ListaVertices::novo(Vetor *v,unsigned char FLAG){
 novo(v->x,v->y,v->z,FLAG);
}

void ListaVertices::novo(Vertice *v,unsigned char FLAG){
 novo(&v->pt,FLAG);
}

void ListaVertices::copia(ListaVertices *lstOrigem){
 Vertice *cvert=lstOrigem->prm;
 while(cvert){
  novo(cvert->pt.x,cvert->pt.y,cvert->pt.z,cvert->FLAG);
  ult->vtex=cvert->vtex;
  ult->mednorm=cvert->mednorm;
  cvert=cvert->prx;
 }
}

void ListaVertices::copia(ListaVerticesM *lstOrigem){
 ptrVerticeM *cvert;
 long i;
 for(i=0,cvert=lstOrigem->items;i<lstOrigem->quant;i++,cvert++){
  novo(cvert->vtm->pt.x,cvert->vtm->pt.y,cvert->vtm->pt.z,cvert->vtm->FLAG);
  ult->vtex=cvert->vtm->vtex;
  ult->mednorm=cvert->vtm->mednorm;
 }
}

VerticeM::VerticeM(){
 FLAG=0;
}

void ListaVerticesM::copia(ListaVerticesM *lstOrigem,bool copia_arestas,bool append){
 ptrVerticeM *v;
 long i,iniindex=append?quant:0;
 Aloca(lstOrigem->quant, append);
 for(i=0,v=lstOrigem->items;i<lstOrigem->quant;i++,v++)
  inivalores(i+iniindex,v->vtm,copia_arestas);
}

void ListaVerticesM::copia(ListaVertices *lstOrigem){
 Vertice *v;
 for(v=lstOrigem->prm;v;v=v->prx){
  novo(&v->pt,v->FLAG);
  last->vtm->vtex=v->vtex;
  last->vtm->mednorm=v->mednorm;
 }
}

bool ListaVerticesM::Aloca(long nitems,bool append){
 long i,iniindex= append?quant:0;
 bool ret=aloca(0,nitems, append);
 
 ptrVerticeM *pvt;
 for(i=iniindex,pvt=items+iniindex;i<(quant);i++,pvt++){
  pvt->vtm=new VerticeM();
 }
 return ret;
}



bool ListaFacesM::Aloca(void *dn,long nitems){
 bool ret=aloca(dn,nitems);
 long i;
 ptrFace *pvt;
 for(i=0,pvt=items;i<quant;i++,pvt++){
  pvt->fc=new Face();
 }
 return ret;
}

bool ListaArestasM::Aloca(void *dn,long nitems){
 bool ret=aloca(dn,nitems);
 long i;
 ptrArestaM *pvt;
 for(i=0,pvt=items;i<quant;i++,pvt++){
  pvt->arm=new ArestaM();
 }
 return ret;
}

bool ListaArestaFacesM::Aloca(void *dn,long nitems){
 bool ret=aloca(dn,nitems);
 long i;
 ptrArestaFace *pvt;
 for(i=0,pvt=items;i<quant;i++,pvt++){
  pvt->loop=new ArestaFace();
 }
 return ret;
}


void ListaVerticesM::apaga(long ind_item){
 if(ind_item>quant-1)return; 
 for(long i=ind_item;i<quant-1;i++){
  *items[i].vtm=*items[i+1].vtm;
 }
 delete items[quant-1].vtm;
 quant--;
 realoca();
}

void ListaVerticesM::inivalores(long indvert,NDOUBLE x,NDOUBLE y,NDOUBLE z,double tu,double tv,unsigned char FLAG){
 ptrVerticeM *vt;
 vt=items+indvert;
 vt->vtm->FLAG=FLAG;
 vt->vtm->pt.flag=FLAG;
 vt->vtm->vtex.u=tu;
 vt->vtm->vtex.v=tv;
 Vetor_detvalfast2(&vt->vtm->pt,x,y,z);
 Vetor_vetornulo(&vt->vtm->mednorm);
 vt->vtm->ptela.detvalfast(0,0);
 vt->vtm->firstar.set(-1);
 vt->vtm->edges.zera();
}

void ListaVerticesM::inivalores(long indvert,Vetor *pt,double tu,double tv,unsigned char FLAG){
 inivalores(indvert,pt->x,pt->y,pt->z,tu,tv,FLAG);
}

void ListaVerticesM::inivalores(long indvert,VerticeM *v,bool copia_arestas){
 inivalores(indvert,&v->pt,v->vtex.u,v->vtex.v,v->FLAG);
 ptrVerticeM *vt;
 vt=items+indvert;
 vt->vtm->vtex.r=v->vtex.r;
 vt->vtm->mednorm=v->mednorm;
 if(copia_arestas){
  vt->vtm->firstar.set(v->firstar);
  vt->vtm->edges.copia(&v->edges);
 }
}

void ListaVerticesM::novo(NDOUBLE x,NDOUBLE y,NDOUBLE z,double tu,double tv,unsigned char FLAG){
 if(!aloca(0))return;
 last->vtm=new VerticeM();
 inivalores(quant-1,x,y,z,tu,tv,FLAG);
}

void ListaVerticesM::novo(VerticeM *v,double tu,double tv,unsigned char FLAG){
 novo(&v->pt,tu,tv,FLAG);
}

void ListaVerticesM::novo(Vetor *v,double tu,double tv,unsigned char FLAG){
 novo(v->x,v->y,v->z,tu,tv,FLAG);
}

ListaVerticesM::ListaVerticesM(){
}

ListaVerticesM::~ListaVerticesM(){
 Limpa();
}

void ListaVerticesM::Limpa(){
 vertices.limpa();
 ptrVerticeM *v;
 long i;
 for(i=0,v=items;i<quant;i++,v++)
  delete v->vtm;
 limpa();
}

bool ListaVerticesM::get_adjacent_edges(Objeto* obj,long indvt,ListaInteiros *lstedges){
 bool vertisborder=get_adjacent_faces( obj, indvt,lstedges);
 if(lstedges->quant==0)return true;
 if(vertisborder){
  long _ii,_jj;
  ptrArestaM *ar;
  Face *lastfc;
  ptrArestaFace *arfc;
  lastfc=(obj->lstfc.items+lstedges->ult->val)->fc;
  for(_ii=0,arfc=lastfc->arestas.items;_ii<lastfc->arestas.quant;_ii++,arfc++){
   if(arfc->loop->a1==lstedges->ult->val2){
    do{
     _jj=arfc->index>0?arfc->index-1:lastfc->arestas.quant-1;
     arfc=lastfc->arestas.items+_jj;
     ar=obj->lstarest.items+arfc->loop->a1;
    }while(ar->arm->p1==ar->arm->p2);
    lstedges->novo(-1,arfc->loop->a1);
    break;
   }
  }
 }
 return vertisborder;
}

bool ListaVerticesM::get_adjacent_facesEX(Objeto* obj,long indvt,ListaInteiros *lstfaces){
 return get_adjacent_faces( obj, indvt,lstfaces);
}

#define find_previous_loop(__fc,__arfc)\
 do{\
 ii=__arfc->index>0?__arfc->index-1:__fc->arestas.quant-1;\
 __arfc=(__fc->arestas.items+ii);\
 ar=(obj->lstarest.items+__arfc->loop->a1)->arm;\
 }while(ar->p1==ar->p2);

#define find_next_loop(__fc,__arfc)\
 do{\
 ii=__arfc->index<__fc->arestas.quant-1?__arfc->index+1:0;\
 __arfc=(__fc->arestas.items+ii);\
 ar=(obj->lstarest.items+__arfc->loop->a1)->arm;\
 }while(ar->p1==ar->p2);

bool ListaVerticesM::get_adjacent_faces(Objeto* obj,long indvt,ListaInteiros *lstfaces){
 lstfaces->limpa();
 ptrArestaM *par;
 ArestaM *ar;
 ptrFace *firstpface,*pfc;
 Vetor v1,v2;
 ptrArestaFace *firstarfc,*oldarfc,*arfc;
 long ii, varindfc,varindarfc;
 bool border=false,needfindloop=false;
 ptrVerticeM *vt=obj->lstvert.items+indvt;
 if(vt->vtm->firstar.ar==-1)return true;
 par=obj->lstarest.items+vt->vtm->firstar.ar;
 ar=par->arm;

 getleftface(varindfc,varindarfc,indvt);
 if(varindfc==-1){
  getrightface(varindfc,varindarfc,indvt);
  needfindloop=true;
  border=true;
 }
 if(varindfc==-1)
  return true;
 pfc=firstpface=obj->lstfc.items+varindfc;
 oldarfc=arfc=firstarfc=(firstpface->fc->arestas.items+varindarfc);
 if(!border){
  do{
   lstfaces->novo(pfc->index,arfc->loop->a1);
   if(lstfaces->quant>50){
    return true;
   }
   find_previous_loop(pfc->fc,arfc);
   ar=(obj->lstarest.items+arfc->loop->a1)->arm;
   getleftface(varindfc,varindarfc,indvt);
   if(varindfc==-1){
    border=true;
    par=obj->lstarest.items+firstarfc->loop->a1;
    ar=par->arm;
    getrightface(varindfc,varindarfc,indvt);
    if(varindfc==-1)
     return true;
    break;
   }
   if(varindfc==pfc->index){
    break;
   }
   pfc=obj->lstfc.items+varindfc;
   arfc=(pfc->fc->arestas.items+varindarfc);
  }
  while(varindfc!=firstpface->index);
 }
 if(border){//v pertence a borda ou crack
  bool valid=true;
  lstfaces->limpa();
  pfc=firstpface;
  arfc=firstarfc;
  int count=0;
  if(needfindloop){
   find_next_loop(pfc->fc,arfc);
   firstarfc=arfc;
  }
  do{
   par=obj->lstarest.items+arfc->loop->a1;
   ar=par->arm;
   getrightface(varindfc,varindarfc,indvt);
   if(varindfc==-1)
    break;
   if(varindfc==pfc->index||count>50){
    valid=false;
    break;
   }
   pfc=obj->lstfc.items+varindfc;
   arfc=(pfc->fc->arestas.items+varindarfc);
   find_next_loop(pfc->fc,arfc);
   count++;
  }
  while(1);
  if(valid){
   vt->vtm->firstar.set(par->index);
   get_adjacent_faces( obj,indvt,lstfaces);
  }
  else
   lstfaces->novo(firstpface->index,firstarfc->loop->a1);
 }
 return border;
}

long ListaVerticesM::jaexisteverts(Objeto *obj,Vetor *p){
 ptrVerticeM *vt;
 long i;
 for(i=0,vt=obj->lstvert.items;i<obj->lstvert.quant;vt++,i++){
  if(__pontosiguais(vt->vtm->pt,*p))
   return i;
 }
 return -1;
}

VerticeM *ListaVerticesM::procuravertice(ListaVerticesM *lstorg,Vetor *p,long *vtret){
 long i;
 ptrVerticeM *vt;
 for(i=0,vt=lstorg->items;i<lstorg->quant;i++,vt++){
  if(__pontosiguais(vt->vtm->pt,*p)){
   if(vtret)*vtret=i;
   return (lstorg->items+i)->vtm;
  }
 }
 return 0;
}

VerticeM *ListaVerticesM::procuravertice(Objeto *obj,Vetor *p,long *vtret){
 return procuravertice(&obj->lstvert,p,vtret);
}

void ListaArestas::revert(ListaArestas *lstar){
 Aresta *prx;
 Vertice *tmp;
 for(Aresta *cur = lstar->prm; cur; cur = prx) {
  prx = cur->prx;
  cur->prx = cur->ant;
  cur->ant = prx;
  tmp=cur->p1;
  cur->p1=cur->p2;
  cur->p2=tmp;
 }
 prx = lstar->prm;
 lstar->prm = lstar->ult;
 lstar->ult = prx;
}

bool ListaArestas::clockwise(ListaArestas *lstar,Vetor *i,Vetor *j,Vetor *k){
 if(!lstar->prm)return false;
 Vetor p,jl,pm;
 int res;
 Vetor_sub(lstar->prm->p2->pt,lstar->prm->p1->pt,p);
 Vetor::pvet(k,&p,&jl);Vetor::normaliza(&jl);
 pm=0.5*(lstar->prm->p2->pt+lstar->prm->p1->pt);
 p=pm+8*EPS*jl;
 res=FuncoesGeometricas::pt_dentro_arestasang_clockwise(lstar,i,j,&p);
 if(res==-1)return false; else if(res==1)return true;
 p=pm-8*EPS*jl;
 res=FuncoesGeometricas::pt_dentro_arestasang_clockwise(lstar,i,j,&p);
 if(res==-1)return false; else if(res==1)return true;
 return false;
}

ListaArestas::ListaArestas(){
 flag=0;
}

void ListaArestas::novo(Vertice *p1,Vertice *p2,unsigned char FLAG){
 if(!p1||!p2)return;
 novoptr();
 atual->p1=p1;
 atual->p2=p2;
 atual->FLAG=FLAG;
 Vetor_soma(p1->pt,p2->pt,atual->centro);
 Vetor_mult(atual->centro,0.5,atual->centro);
}

void ListaArestas::novo(ListaVertices *verts,Vertice *p1,Vertice *p2,unsigned char FLAG)
{
 verts->novo(p1);
 verts->novo(p2);
 novo(verts->ult->ant,verts->ult,FLAG);
}

void ListaArestas::inseredepois(Aresta *anterior,Vertice *p1,Vertice *p2,unsigned char FLAG){
 if(!p1||!p2)return;
 novoptrafter(anterior);
 atual->p1=p1;
 atual->p2=p2;
 atual->FLAG=FLAG;
 Vetor_soma(p1->pt,p2->pt,atual->centro);
 Vetor_mult(atual->centro,0.5,atual->centro);
}

ListaArestas::~ListaArestas(){
 Limpa();
}

void ListaArestas::Limpa(){
 arestasH.limpa();
 arestasV.limpa();
 limpa();
}

void ListaArestas::copia(ListaVertices *lstvert,ListaArestas *lstOrigem){
 Aresta *ar;
 Vertice *v1,*v2,*vert;
 ar=lstOrigem->prm;
 while(ar){
  v1=0;v2=0;
  vert= lstvert->prm;
  while(vert){
   if(vetoresiguais(&vert->pt,&ar->p1->pt,EPS))
    v1=vert;
   else if(vetoresiguais(&vert->pt,&ar->p2->pt,EPS))
    v2=vert;
   if(v1!=0&&v2!=0){
    novo(v1,v2,1);
    ult->aux.copia(&ar->aux);
    ult->FLAG=ar->FLAG;
    break;
   }
   vert=vert->prx;
  }
  ar=ar->prx;
 }
}

void ListaArestas::copia(ListaVertices *lstvert,ListaArestasM *lstOrigem){
 ptrArestaM *ar;
 ListaVerticesM *lstvertorg=(ListaVerticesM *)lstOrigem->dono;
 long i;
 Vertice *v1,*v2,*vert;
 for(i=0,ar=lstOrigem->items;i<lstOrigem->quant;i++,ar++){
  v1=0;v2=0;
  vert= lstvert->prm;
  while(vert){
   if(vetoresiguais(&vert->pt,&(lstvertorg->items+ar->arm->p1)->vtm->pt,EPS))
    v1=vert;
   else if(vetoresiguais(&vert->pt,&(lstvertorg->items+ar->arm->p2)->vtm->pt,EPS))
    v2=vert;
   if(v1!=0&&v2!=0){
    novo(v1,v2,1);
    ult->aux.copia(&ar->arm->aux);
    ult->FLAG=ar->arm->FLAG;
    break;
   }
   vert=vert->prx;
  }
 }
}

void ListaArestasM::revert(ListaArestasM *lstar){
 long niter=lstar->quant/2,i,tp;
 ArestaM tind;
 for(i=0;i<niter;i++){
  tind=*lstar->items[lstar->quant-1-i].arm;
  *lstar->items[lstar->quant-1-i].arm=*lstar->items[i].arm;
  tp=lstar->items[lstar->quant-1-i].arm->p1;lstar->items[lstar->quant-1-i].arm->p1=lstar->items[lstar->quant-1-i].arm->p2;
  lstar->items[lstar->quant-1-i].arm->p2=tp;
  *lstar->items[i].arm=tind;
  tp=lstar->items[i].arm->p1;lstar->items[i].arm->p1=lstar->items[i].arm->p2;
  lstar->items[i].arm->p2=tp;
 }
 if((niter%2)){
  tp=lstar->items[i].arm->p1;lstar->items[i].arm->p1=lstar->items[i].arm->p2;
  lstar->items[i].arm->p2=tp;
 }
}

FVOID ListaArestasM::centroide(ListaArestasM *lstar,ListaVerticesM *lstvert,Vetor *dest){
 Vetor_vetornulo(dest);
 for(long ar=0;ar<lstar->quant;ar++){
  if(lstar->items[ar].arm->FLAG&AREST_INVERTIDA)
   *dest=*dest+lstvert->items[lstar->items[ar].arm->p2].vtm->pt;
  else
   *dest=*dest+lstvert->items[lstar->items[ar].arm->p1].vtm->pt;
 }
 if(lstar->quant)
  Vetor_div(*dest,(NDOUBLE)lstar->quant,*dest);
}

char ListaArestasM::jaexistearesta(Objeto *obj,long v1,long v2,long *indarest){
 long i;
 ptrArestaM *ar;
 InteiroLight *iil;
 ptrVerticeM *pv[2]={obj->lstvert.items+v1,obj->lstvert.items+v2};
 for(i=0;i<2;i++)
  for(iil=pv[i]->vtm->edges.prm;iil;iil=iil->prx){
   ar=obj->lstarest.items+iil->val;
   if(ar->arm->p1==v1&&ar->arm->p2==v2){
    if(indarest)*indarest=ar->index;
    return POSITIVA;
   }
   else if(ar->arm->p1==v2&&ar->arm->p2==v1){
    if(indarest)*indarest=ar->index;
    return NEGATIVA;
   }
  }
  return 0;
}

char ListaArestasM::jaexistearesta(Objeto *obj,long v1,long v2,ptrArestaM **arres,long *indarest){
 if(arres)*arres=0;
 long i;
 ptrArestaM *ar;
 for(i=0, ar=obj->lstarest.items;i<obj->lstarest.quant;i++,ar++){
  if(ar->arm->p1==v1&&ar->arm->p2==v2){
   if(arres)*arres=ar;
   if(indarest)*indarest=i;
   return POSITIVA;
  }
  else if(ar->arm->p1==v2&&ar->arm->p2==v1){
   if(arres)*arres=ar;
   if(indarest)*indarest=i;
   return NEGATIVA;
  }
 }
 return 0;
}

ListaArestasM::ListaArestasM(){
 flag=0;
}

ListaArestasM::~ListaArestasM(){
 Limpa();
}

void ListaArestasM::Limpa(){
 arestasH.limpa();
 arestasV.limpa();
 ptrVerticeM *v1,*v2;
 ListaVerticesM *plstvert=(ListaVerticesM*)this->dono;
 long i;
 ptrArestaM *ar;
 if(items){
  for(i=0,ar=items;i<quant;i++,ar++){
   ar->arm->aux.limpa();
   v1=plstvert->items+ar->arm->p1;
   v2=plstvert->items+ar->arm->p2;
   if(v1->vtm->firstar.ar==ar->index)
    v1->vtm->firstar.set(-1);
   if(v2->vtm->firstar.ar==ar->index)
    v2->vtm->firstar.set(-1);
   v1->vtm->edges.apagabyval(ar->index);
   v2->vtm->edges.apagabyval(ar->index);
   delete ar->arm;
  }
 }
 limpa();
}

void ListaArestasM::copia(ListaVerticesM *owner,ListaArestasM *lstOrigem,long iniindex,long fciniindex,bool copyfacesptr){
 long i;
 ptrArestaM *arorg;
 for(i=0,arorg=lstOrigem->items;i<lstOrigem->quant;i++,arorg++){
  novo(owner,iniindex+arorg->arm->p1,iniindex+arorg->arm->p2,1);
  last->arm->aux.copia(&arorg->arm->aux);
  if(copyfacesptr)
   last->arm->vizinfo.Set(arorg->arm->vizinfo.fe+fciniindex,arorg->arm->vizinfo.fd+fciniindex,arorg->arm->vizinfo.arfce,arorg->arm->vizinfo.arfcd);
  last->arm->FLAG=arorg->arm->FLAG;
 }
}

void ListaArestasM::copia(ListaVerticesM *owner,ListaArestas *lstOrigem,ListaVertices *lstvertOrigem){
 Aresta *ar;
 for(ar=lstOrigem->prm;ar;ar=ar->prx){
  novo(owner,lstvertOrigem->RetornaIndicePorItem(ar->p1),lstvertOrigem->RetornaIndicePorItem(ar->p2),ar->FLAG);
 }
}

long ListaArestasM::apaga(long ind_item){
 if(ind_item>quant-1)return 0;
 if(quant==0)return 0;
 items[ind_item].arm->aux.limpa();
 long i;
 VerticeM *vt;
 ListaVerticesM *lstvert=(ListaVerticesM*)dono;
 for(i=0;i<quant-1;i++){
  vt=(lstvert->items+items[i].arm->p1)->vtm;
  vt->edges.apagabyval(ind_item);
  if(vt->firstar.ar==ind_item)
   vt->firstar.ar=vt->edges.prm?vt->edges.prm->val:-1;
  vt=(lstvert->items+items[i].arm->p2)->vtm;
  vt->edges.apagabyval(ind_item);
  if(vt->firstar.ar==ind_item)
   vt->firstar.ar=vt->edges.prm?vt->edges.prm->val:-1;
 }
 for( i=ind_item;i<quant-1;i++){    
  *items[i].arm=*items[i+1].arm;
  (items+i)->index=i;
 }
 delete items[quant-1].arm;
 quant--;
 realoca();
 if(quant==0)return 0;
 if(ind_item==quant)
  return quant-1;
 else
  return ind_item;
}

void ListaArestasM::apaga(ArestaM *ar){
 bool achou=false;
 long i;
 for( i=0;i<quant;i++){
  if((items+i)->arm->p1==ar->p1&&(items+i)->arm->p2==ar->p2){
   achou =true;
   break;
  }
 }
 if(achou)
  apaga(i);
}

void ListaArestasM::zeralistas(){
 long i;
 ptrArestaM *ar;
 for(i=0,ar=items;i<quant;i++,ar++){
  ar->arm->vizinfo.Set(-1,-1,-1,-1);
  ar->arm->aux.zera();
 }
}

void ListaArestasM::inivalores(long indaresta,long p1,long p2,unsigned char FLAG){
 ListaVerticesM *owner=(ListaVerticesM *)dono;
 ptrArestaM *ar=items+indaresta;
 ptrVerticeM *v1,*v2;
 ar->arm->p1=p1;
 ar->arm->p2=p2;
 ar->index=indaresta;
 ar->arm->FLAG=FLAG;
 ar->arm->vizinfo.Set(-1,-1,-1,-1);
 ar->arm->aux.zera();
 v1=owner->items+p1;
 v2=owner->items+p2;
 Vetor::midpoint(&v1->vtm->pt,&v2->vtm->pt,&ar->arm->centro);
 if(p1!=p2){
  v1->vtm->edges.novo(ar->index);
  v1->vtm->firstar.ar=ar->index;
  v2->vtm->edges.novo(ar->index);
  v2->vtm->firstar.ar=ar->index;
 }
}

void ListaArestasM::novo(ListaVerticesM *owner,long p1,long p2,unsigned char FLAG){
 if(p1==-1||p2==-1)return;
 if(!aloca(owner))return;
 last->arm=new ArestaM();
 inivalores(quant-1,p1,p2,FLAG);
}

void ListaArestasM::novo(ListaVerticesM *owner,VerticeM *p1,VerticeM *p2,unsigned char FLAG){
 ((Objeto*)owner)->lstvert.novo(p1);
 ((Objeto*)owner)->lstvert.novo(p2);
 novo(owner,((Objeto*)owner)->lstvert.quant-2,((Objeto*)owner)->lstvert.quant-1,FLAG);
}

void ListaArestaFacesM::Limpa(){
 if(dono){
  ArestaM *ar;
  ptrArestaFace *arfc;
  long i;
  Objeto *lstobjowner=(Objeto*)((ListaFacesM*)dono)->dono;
  ListaArestasM *lstar=(ListaArestasM*)&lstobjowner->lstarest;
  for(i=0,arfc=items;i<quant;i++,arfc++){
   ar=(lstar->items+arfc->loop->a1)->arm;
   delete arfc->loop;
   /*if(ar->vizinfo.fe==arfc->face)
   ar->vizinfo.fe=-1;
   if(ar->vizinfo.fd==arfc->face)
   ar->vizinfo.fd=-1;*/

  }
 }
 limpa();
}

void ListaArestaFacesM::revert(){
 long niter=quant/2,i;
 ArestaFace tind;
 for(i=0;i<quant;i++){
  if(i<niter){
   tind=*items[quant-1-i].loop;
   *items[quant-1-i].loop=*items[i].loop;
   *items[i].loop=tind;
  }
  items[i].loop->FLAG^=AREST_INVERTIDA;
 }
}

ListaArestaFacesM::~ListaArestaFacesM(){
 Limpa();
}


void ListaArestaFacesM::copia(ListaFacesM *__owner,long facedest,ListaArestaFacesM *lstOrigem,long iniindex){
 long i;
 ptrArestaFace *arfc;
 for(i=0,arfc=lstOrigem->items;i<lstOrigem->quant;i++,arfc++)
  nova(__owner,iniindex+arfc->loop->a1,facedest,arfc->loop->FLAG,arfc->loop->textura_st.u,arfc->loop->textura_st.v);
}

void ListaArestaFacesM::apaga(int ind_item){ 
 for(long i=ind_item;i<quant-1;i++){  
  *items[i].loop=*items[i+1].loop;
  (items+i)->index=i;
 }
 delete items[quant-1].loop;
 quant--;
 realoca();
}

void ListaArestaFacesM::inivalores(long indarfc,long a1,long face,unsigned char FLAG,NDOUBLE s,NDOUBLE t){
 Objeto *lstobjowner=(Objeto*)((ListaFacesM*)dono)->dono;
 ListaArestasM *lstar=(ListaArestasM*)&lstobjowner->lstarest;
 ListaVerticesM *plstvert=(ListaVerticesM*)&lstobjowner->lstvert;
 ptrArestaFace *parfc=items+indarfc;
 ArestaFace *arfc=parfc->loop;
 ptrArestaM *par=lstar->items+a1;
 ArestaM *ar=par->arm;
 arfc->a1=a1;
 parfc->index=indarfc;
 arfc->face=face;
 arfc->textura_st.u=s;
 arfc->textura_st.v=t;
 arfc->FLAG=FLAG;
 long vtest1,vtest2;
 ptrVerticeM *_v2;
 getptsfromarfc(vtest1,vtest2,arfc,ar);
 if(vtest1!=vtest2){
  _v2=plstvert->items+vtest1;
  if(_v2->vtm->firstar.ar==-1)
   _v2->vtm->firstar.set(par->index);
  if(vtest1==ar->p1){//ar->vizinfo.fe==-1&&
   ar->vizinfo.fe=face;
   ar->vizinfo.arfce=parfc->index;
  }
  if(vtest1==ar->p2){//ar->vizinfo.fd==-1&&
   ar->vizinfo.fd=face;
   ar->vizinfo.arfcd=parfc->index;
  }
 }
}

void ListaArestaFacesM::nova(ListaFacesM *owner,long a1,long face,unsigned char FLAG,NDOUBLE s,NDOUBLE t){
 if(!aloca(owner))return;
 last->loop=new ArestaFace();
 inivalores(quant-1,a1,face,FLAG,s,t);
}

bool ListaArestaFacesM::clockwise(ListaArestaFacesM *lstar,Vetor *i,Vetor *j,Vetor *k){
 if(!lstar->quant||!lstar->dono)return false;
 ArestaFace *arfc=lstar->items->loop;
 Objeto *lstobjowner=(Objeto*)((ListaFacesM*)lstar->dono)->dono;
 Vetor p,jl,pm;
 long res,ip1,ip2;
 ip1=lstobjowner->lstarest.items[arfc->a1].arm->p1;
 ip2=lstobjowner->lstarest.items[arfc->a1].arm->p2;
 Vetor_sub(lstobjowner->lstvert.items[ip2].vtm->pt,lstobjowner->lstvert.items[ip1].vtm->pt,p);
 Vetor::pvet(k,&p,&jl);Vetor::normaliza(&jl);
 pm=0.5*(lstobjowner->lstvert.items[ip2].vtm->pt+lstobjowner->lstvert.items[ip1].vtm->pt);
 p=pm+8*EPS*jl;
 res=FuncoesGeometricas::pt_dentro_arestasang_clockwise_fc(lstar,i,j,&p);
 if(res==-1)return false; else if(res==1)return true;
 p=pm-8*EPS*jl;
 res=FuncoesGeometricas::pt_dentro_arestasang_clockwise_fc(lstar,i,j,&p);
 if(res==-1)return false; else if(res==1)return true;
 return false;
}

void ListaFacesM::apaga(long ind_item){
 if(ind_item>quant-1)return;
 items[ind_item].fc->arestas.Limpa();
 long i;
 ptrFace *fc;
 for(i=ind_item,fc=items+ind_item;i<quant-1;i++,fc++){
  *(fc->fc)=*((fc+1)->fc);
  fc->index=i;
  fc->fc->arestas.Limpa();
  fc->fc->arestas.copia(this,i,&(fc+1)->fc->arestas,0);
 }
 delete items[quant-1].fc;
 quant--;
 realoca();
}

void ListaFacesM::calcnormalface(Objeto *obj,Face *face){
 int i;
 Vetor v1,v2,va;
 ptrVerticeM *p1,*p2,*p1prx,*p2prx;
 ptrArestaFace *ar,*arprx;
 for(i=0,ar=face->arestas.items;i<face->arestas.quant-1;i++,ar++){
  arprx=ar+1;
  p1=obj->lstvert.items+(obj->lstarest.items+ar->loop->a1)->arm->p1;p2=obj->lstvert.items+(obj->lstarest.items+ar->loop->a1)->arm->p2;
  p1prx=obj->lstvert.items+(obj->lstarest.items+arprx->loop->a1)->arm->p1;p2prx=obj->lstvert.items+(obj->lstarest.items+arprx->loop->a1)->arm->p2;
  v1=p1->vtm->pt+RANDR(.15,.85)*(p2->vtm->pt-p1->vtm->pt);//randomico para evitar erros qdo testar ponto dentro face
  v2=p1prx->vtm->pt+RANDR(.15,.85)*(p2prx->vtm->pt-p1prx->vtm->pt);
  Vetor_sub(v2,v1,face->von_i);
  if(!vetorigualazero(face->von_i))
   break;
 }
 bool normalok=false,concavetestok=false;
 Vetor normalat,normalant;
 for(i=0,ar=face->arestas.items;(i<face->arestas.quant)&&(!normalok||!concavetestok);i++,ar++){
  arprx=i<face->arestas.quant-1?ar+1:face->arestas.items;
  if(ar->loop->FLAG&AREST_INVERTIDA){
   p2=obj->lstvert.items+(obj->lstarest.items+ar->loop->a1)->arm->p1;p1=obj->lstvert.items+(obj->lstarest.items+ar->loop->a1)->arm->p2;}
  else{
   p1=obj->lstvert.items+(obj->lstarest.items+ar->loop->a1)->arm->p1;p2=obj->lstvert.items+(obj->lstarest.items+ar->loop->a1)->arm->p2;}
  if(arprx->loop->FLAG&AREST_INVERTIDA){
   p2prx=obj->lstvert.items+(obj->lstarest.items+arprx->loop->a1)->arm->p1;p1prx=obj->lstvert.items+(obj->lstarest.items+arprx->loop->a1)->arm->p2;}
  else{
   p1prx=obj->lstvert.items+(obj->lstarest.items+arprx->loop->a1)->arm->p1;p2prx=obj->lstvert.items+(obj->lstarest.items+arprx->loop->a1)->arm->p2;}

  Vetor_sub(p2prx->vtm->pt,p1prx->vtm->pt,v1);
  Vetor_sub(p1->vtm->pt,p2->vtm->pt,v2);
  Vetor::pvet(&v1,&v2,&normalat);
  if(!normalok){
   Vetor_detvalfast(&face->normal,&normalat);
   if(!vetorigualazero(face->normal))
    normalok=true;
  }
  if(!concavetestok){
   if(i>0){
    concavetestok=__DOT(normalat,normalant)<0.0;
   }
   if(concavetestok){
    face->FLAG|=FACE_ISCONCAVE;   
   }
  }
  Vetor_detvalfast(&normalant,&normalat);
 }
 Vetor::normaliza(&face->von_i);
 Vetor::normaliza(&face->normal);
 Vetor::pvet(&face->normal,&face->von_i,&face->von_j);
 if(ListaArestaFacesM::clockwise(&face->arestas,&face->von_i,&face->von_j,&face->normal)){
  face->normal=-1.0*face->normal;
  Vetor::pvet(&face->normal,&face->von_i,&face->von_j);
 }
}

void ListaFacesM::updateparamsface(Objeto *obj,Face *fc){
 long iarfc,ivprm;
 Vetor vtmp,vt1,vt2;
 ArestaM *ar;
 ptrVerticeM *v1,*v2,*vprm;
 ptrArestaFace *arfc;
 Vetor_vetornulo(&fc->centroide);
 calcnormalface(obj,fc);
 arfc=fc->arestas.items; 
 if(!arfc)return;
 ar=(obj->lstarest.items+arfc->loop->a1)->arm;
 getp1fromarfc(ivprm,arfc->loop,ar);
 vprm=obj->lstvert.items+ivprm;

 for(iarfc=0,arfc=fc->arestas.items;iarfc<fc->arestas.quant;iarfc++,arfc++){
  ar=(obj->lstarest.items+arfc->loop->a1)->arm;
  v1=obj->lstvert.items+ar->p1;
  v2=obj->lstvert.items+ar->p2;
  if(arfc->loop->FLAG&AREST_INVERTIDA){
   Vetor_soma(fc->centroide,v2->vtm->pt,fc->centroide);
   Vetor_sub(v2->vtm->pt,vprm->vtm->pt,vtmp);
   Vetor_sub(v1->vtm->pt,v2->vtm->pt,vt1);
  }
  else{
   Vetor_sub(v1->vtm->pt,vprm->vtm->pt,vtmp);
   Vetor_sub(v2->vtm->pt,v1->vtm->pt,vt1);
   Vetor_soma(fc->centroide,v1->vtm->pt,fc->centroide);
  }
  Vetor::detvalfast(&arfc->loop->coordlocal,__DOT(vtmp,fc->von_i),__DOT(vtmp,fc->von_j));
 }
 Vetor_div(fc->centroide,(NDOUBLE)fc->arestas.quant,fc->centroide);
}

void ListaFacesM::Limpa(){
 ptrFace *fc;
 long indf;
 for(fc=items,indf=0;indf<quant;indf++,fc++){
  fc->fc->arestas.Limpa();
  delete fc->fc;
 }
 limpa();
}

void ListaFacesM::copia(Objeto *owner,ListaFacesM *lstOrigem,long ariniindex){
 ptrFace *fc;
 long i;
 for(i=0,fc=lstOrigem->items;i<lstOrigem->quant;i++,fc++) {
  nova(owner,fc->fc->FLAG);
  last->fc->centroide=fc->fc->centroide;
  last->fc->normal=fc->fc->normal;
  last->fc->arestas.copia(&owner->lstfc,quant-1,&fc->fc->arestas,ariniindex);
 }
}

void ListaFacesM::inivalores(long indfc,unsigned char FLAG){
 Face *fc=(items+indfc)->fc;
 fc->FLAG=FLAG;
 fc->normal.flag=fc->von_i.flag=fc->von_j.flag=VETOR;
 fc->arestas.zera();
 fc->mat=0;
 fc->bkmat=0;
 (items+indfc)->index=indfc;
}

void ListaFacesM::nova(Objeto *owner,unsigned char FLAG){
 if(!aloca(owner))return;
 last->fc=new Face();
 inivalores(quant-1,FLAG);
}

PropObj::PropObj(Objeto *_obj){
 valor=0;
 obj=_obj;
 tipo=PROPINDETERMINADA;
}

void PropObj::limpa(PropObj *prop){
 if(!prop)return;
 if(prop->tipo==PROPARVORECSG){
  if(prop->valor)
   delete (CSGTREE*)prop->valor;
 }
 else if(prop->tipo==PROPLSTOBJS){
  static int ct=0;
  if(ct==51)
  {
   ct=0;
  }
  ct++;
  if(prop->valor)
   delete (ListaObjetos*)prop->valor;
 }
 else if(prop->tipo==PROPARRAYDB){
  if(prop->valor)
   delete (lstreais*)prop->valor;
 }
 else if(prop->tipo==PROPnDoubles){
  if(prop->valor)
   delete (lstnDoubles*)prop->valor;
 }
 else if(prop->tipo==PROPLSTBOUNDARIES){
  if(prop->valor)
   delete (ListaHatchBoundaries*)prop->valor;
 }
 else if(prop->tipo==PROPLSTPALAVRAS){
  if(prop->valor)
   delete (LstPalavras*)prop->valor;
 }
 else if(prop->tipo==PROPLSTLUZES){
  if(prop->valor)
   delete (ListaLuzes*)prop->valor;
 }
 else if(prop->tipo==PROPEIXOSCOORD){
  if(prop->valor)
   delete []((Vetor*)prop->valor);
 }
 else if(prop->tipo==PROPARQIMGEM){
  if(prop->valor)
   delete ((Imagem*)prop->valor);
 }
 else if(prop->tipo==PROPCAMERA){
 }
 else if(prop->valor&&prop->tipo!=PROPPTR&&prop->tipo!=PROPMATERIAL&&prop->tipo!=PROPFONTETEXTO&&prop->tipo!=PROPTIPOLIN&&prop->tipo!=PROPHATCHPATTERNS&&prop->tipo!=PROPPTRBLOCO&&prop->tipo!=PROPDIMESTILO)
  free(prop->valor);
 prop->valor=0;
}

PropObj::~PropObj(){
 limpa(this);
}

bool PropObj::dettipo(PropObj *dest,TipoPropObj tipo){
 if(dest->tipo==PROPINDETERMINADA)
  dest->tipo=tipo;
 else if(dest->tipo!=tipo){
  FuncUtils::erroMsg("verifique tipo");
  return false;
 }
 return true;
}

void PropObj::atribuiarvorecsg(PropObj *prop,void *val,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPARVORECSG))
  return;
 prop->valor=val;
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

void PropObj::atribui(PropObj *prop,void *val,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPPTR))return;
 prop->valor=val;
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

void PropObj::atribui(PropObj *prop,FonteDef *val,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPFONTETEXTO))return;
 prop->valor=val;
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

void PropObj::atribui(PropObj *prop,ListaHatchPatterns *val,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPHATCHPATTERNS))return;
 prop->valor=val;
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

void PropObj::atribui(PropObj *prop,ListaHatchBoundaries *val,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPLSTBOUNDARIES))return;
 prop->valor=val;
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

void PropObj::atribui(PropObj *prop,LstPalavras *val,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPLSTPALAVRAS))return;
 prop->valor=val;
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

void PropObj::atribui(PropObj *prop,lstnDoubles *val,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPnDoubles))return;
 prop->valor=val;
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

void PropObj::atribui(PropObj *prop,ListaObjetos *val,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPLSTOBJS))return;
 prop->valor=val;
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

void PropObj::atribui(PropObj *prop,ListaLuzes *val,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPLSTLUZES))return;
 prop->valor=val;
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

void PropObj::atribui(PropObj *prop,LinEstilo *val,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPTIPOLIN))return;
 prop->valor=val;
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}


void PropObj::atribui(PropObj *prop,Material *val,Objeto *obj,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPMATERIAL))return;
 prop->valor=val;
 if(obj&&(obj->flag&OBJCANPUTINMATLIST))
  val->objs.novo((void*)obj);
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

 void PropObj::atribui(PropObj *prop,Camera *val,bool addtoundo){
  if(!prop)return;
  if(!dettipo(prop,PROPCAMERA))return;
  prop->valor=val;
  if(addtoundo)
   janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
 }

void PropObj::atribui(PropObj *prop,Bloco *val,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPPTRBLOCO))return;
 prop->valor=val;
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

lstreais::lstreais()
{
 val=0;
}

lstreais::~lstreais(){
 if(val)free(val);
}

void PropObj::adicionareal(PropObj *prop,double val){
 if(!prop||!dettipo(prop,PROPARRAYDB))
  return;
 int tam=1;
 if(prop->valor)
  tam=((lstreais*)prop->valor)->quant+1;
 else
  prop->valor=new lstreais();
 REALLOCVOID(((lstreais*)prop->valor)->val,double,tam*sizeof(double));
 ((lstreais*)prop->valor)->val[tam-1]=val;
 ((lstreais*)prop->valor)->quant=tam;
}

void PropObj::adicionareal(PropObj *prop,double val,int onde){
 if(!prop||!dettipo(prop,PROPARRAYDB))
  return;
 int tam=1;
 if(prop->valor)
  tam=((lstreais*)prop->valor)->quant+1;
 else
  prop->valor=new lstreais();
 if(onde>((lstreais*)prop->valor)->quant||onde<0)
  return;
 REALLOCVOID(((lstreais*)prop->valor)->val,double,tam*sizeof(double));
 memcpy(((lstreais*)prop->valor)->val+onde+1,((lstreais*)prop->valor)->val+onde,(((lstreais*)prop->valor)->quant-onde)*sizeof(double));
 ((lstreais*)prop->valor)->val[onde]=val;
 ((lstreais*)prop->valor)->quant=tam;
}

void PropObj::atribuireal(PropObj *prop,double val,int ind){
 if(!prop||!dettipo(prop,PROPARRAYDB)||!prop->valor)
  return;
 if(ind>((lstreais*)prop->valor)->quant-1||ind<0)
  return;
 ((lstreais*)prop->valor)->val[ind]=val;
}

void PropObj::apagareal(PropObj *prop,int ind){
 if(!prop||!dettipo(prop,PROPARRAYDB)||!prop->valor)
  return;
 if(ind>((lstreais*)prop->valor)->quant-1||ind<0)
  return;

 if(ind<((lstreais*)prop->valor)->quant-1)
  memcpy(((lstreais*)prop->valor)->val+ind,((lstreais*)prop->valor)->val+ind+1,(((lstreais*)prop->valor)->quant-1-ind));
 int tam;
 tam=((lstreais*)prop->valor)->quant-1;
 REALLOCVOID(((lstreais*)prop->valor)->val,double,tam*sizeof(double));
 ((lstreais*)prop->valor)->quant=tam;
}

void PropObj::atribui(PropObj *prop,unsigned int val,bool addtoundo){
 if(!prop||!dettipo(prop,PROPUINT))
  return;
 if(!prop->valor)
  ALLOCVOID(prop->valor,unsigned int,1);
 *((unsigned int*)prop->valor)=val;
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

void PropObj::atribui(PropObj *prop,int val,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPINTEIRO))
  return;
 if(!prop->valor)
  ALLOCVOID(prop->valor,int,1);
 *((int*)prop->valor)=val;

 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

void PropObj::atribui(PropObj *prop,NDOUBLE val,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPREAL))
  return;
 if(!prop->valor)
  ALLOCVOID(prop->valor,NDOUBLE,1);
 *((NDOUBLE*)prop->valor)=val;
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

void PropObj::atribui(PropObj *prop,const char *val,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPSTRING))
  return;
 if(prop->valor)
  free(prop->valor);
 ALLOCVOID(prop->valor,char,n_strlen(val)+1);
 n_strcpy(((char*)prop->valor),val);
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

void PropObj::atribui(PropObj *prop,char *val,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPSTRING))
  return;
 if(prop->valor)
  free(prop->valor);

 ALLOCVOID(prop->valor,char,n_strlen(val)+1);
 n_strcpy(((char*)prop->valor),val);
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

bool PropObj::atribuiimagem(PropObj *prop,char *val,bool addtoundo){
 if(!prop)return false;
 if(!dettipo(prop,PROPARQIMGEM))
  return false;
 if(prop->valor)
  delete (Imagem*)prop->valor;
 NEW2(prop->valor,Imagem,1);
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
 return ((Imagem*)prop->valor)->abre(val);
}



void PropObj::copia(PropObj *dest,PropObj *origem,Objeto *objdest){
 switch(origem->tipo){
 case PROPLSTOBJS:{
  ListaObjetos *lst=new ListaObjetos(),*org=(ListaObjetos*)origem->valor;
  PropObj::atribui(dest,lst);
  lst->clone(org);
                  }
                  break;
 case PROPARVORECSG:{
  CSGTREE *arvore=new CSGTREE;
  PropObj::atribuiarvorecsg(dest,arvore);
  arvore->opcoes= ((CSGTREE*)origem->valor)->opcoes;
  CSGTREE::copia(arvore,((CSGTREE*)origem->valor)->prm,0,0,0);
                    }
                    break;
 case PROPEIXOSCOORD:
  atribuieixo(dest,((Vetor*)origem->valor)[0],((Vetor*)origem->valor)[1], ((Vetor*)origem->valor)[2]);
  break;
 case PROPVETOR3D:
  atribuivt(dest,((Vetor*)origem->valor)->x,((Vetor*)origem->valor)->y,((Vetor*)origem->valor)->z);
  break;
 case PROPPONTO3D:
  atribuipt(dest,((Vetor*)origem->valor)->x,((Vetor*)origem->valor)->y,((Vetor*)origem->valor)->z);
  break;
 case PROPPTR:
  atribui(dest,origem->valor);
  break;
 case PROPMATERIAL:
  atribui(dest,((Material*)origem->valor),objdest);
  break;
 case PROPPTRBLOCO:
  atribui(dest,((Bloco*)origem->valor));
  break;
 case PROPARQIMGEM:
  atribuiimagem(dest,((Imagem*)origem->valor)->path);
  break;
 case PROPTIPOLIN:
  atribui(dest,((LinEstilo*)origem->valor));
  break;
 case PROPFONTETEXTO:
  atribui(dest,((FonteDef*)origem->valor));
  break;
 case PROPINTEIRO:case PROPBOOL:case PROPTEXTURA:
 case PROPTIPOSPL:case PROPESTILOTEXTO:case PROPESTILOTEXTO3D:case PROPALINTEXTO:
 case PROPTIPOCOTA:case PROPORIENTCOTA:case PROPTIPOSETACOTA:
 case PROPTIPOESFERA:case PROPTIPOCSG:case PROPTIPOTERRAIN:
 case PROPTIPOPOLIGONO:case PROPTIPOSUBDIVISION:
  atribui(dest,*((int*)origem->valor));
  break;
 case PROPUINT:case PROPCOR:case PROPCOROBJ:
  atribui(dest,*((unsigned int*)origem->valor));
  break;
 case PROPREAL:case PROPANGULO:
  atribui(dest,*((NDOUBLE*)origem->valor));
  break;
 case PROPSTRING:
  atribui(dest,((char*)origem->valor));
  break;
 case PROPLSTPALAVRAS:
  atribui(dest,new LstPalavras);
  LstPalavras::copia((LstPalavras*)dest->valor,(LstPalavras*)origem->valor);
  break;
 case PROPARRAYDB:{
  int i;
  for(i=0;i<((lstreais*)origem->valor)->quant;i++)
   adicionareal(dest,((lstreais*)origem->valor)->val[i]);
                  }
                  break;
 case PROPnDoubles:{
  atribui(dest,new lstnDoubles);
  lstnDoubles::clone((lstnDoubles*)dest->valor,(lstnDoubles*)origem->valor);
 }
                  break;
 default:
  break;
 }
 dest->tipo=origem->tipo;
}


void PropObj::getpt(Vetor *res,PropObj *prop){
 Vetor_detvalfast2(res,((Vetor*)prop->valor)->x,((Vetor*)prop->valor)->y,((Vetor*)prop->valor)->z);
}

Vetor PropObj::getpt(PropObj *prop){
 return *((Vetor*)prop->valor);
}

void PropObj::atribuipt(PropObj *prop,NDOUBLE x,NDOUBLE y,NDOUBLE z,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPPONTO3D))
  return;
 if(!prop->valor)
  ALLOCVOID(prop->valor,Vetor,1);
 Vetor_detvalfast2(((Vetor*)prop->valor),x,y,z);
 
 ((Vetor*)prop->valor)->flag=VPONTO;
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}

void PropObj::atribuipt(PropObj *prop,Vetor &v,bool addtoundo){
 atribuipt(prop,v.x,v.y,v.z,addtoundo);
}

PropObj* PropObj::Aloca(int quant,Objeto *obj){
 PropObj* prop=new PropObj[quant];
 for(int i=0;i<quant;i++)
  prop[i].obj=obj; 
 return prop;
}

Vetor *PropObj::geteixos(PropObj *prop){
 return ((Vetor*)prop->valor);
}

void PropObj::atribuieixo(PropObj *prop,const Vetor &e1,const Vetor &e2,const  Vetor &e3){
 if(!prop)return;
 if(!dettipo(prop,PROPEIXOSCOORD))
  return;
 if(!prop->valor)
  prop->valor=(void*)(new Vetor[3]);
 ((Vetor*)prop->valor)[0]=e1;
 ((Vetor*)prop->valor)[0].flag=VETOR;
 ((Vetor*)prop->valor)[1]=e2;
 ((Vetor*)prop->valor)[1].flag=VETOR;
 ((Vetor*)prop->valor)[2]=e3;
 ((Vetor*)prop->valor)[2].flag=VETOR;

}

void PropObj::atribuivt(PropObj *prop,NDOUBLE x,NDOUBLE y,NDOUBLE z,bool addtoundo){
 if(!prop)return;
 if(!dettipo(prop,PROPVETOR3D))
  return;
 if(!prop->valor)
  ALLOCVOID(prop->valor,Vetor,1);
 Vetor_detvalfast2(((Vetor*)prop->valor),x,y,z);
 ((Vetor*)prop->valor)->flag=VETOR;
 if(addtoundo)
  janatbasepanel.lstundorec.novalistaundo(0,UNDO_ALTERACAO_PROPS_OBJETO,UNDO,prop,true);
}


Objeto::Objeto(JanelaFilhaBase *jfb){
 camada=0;
 if(jfb){
  camada=jfb->camadas.highlander;
  for(Viewport* vp=jfb->viewports.prm;vp;vp=vp->prx){
   borders_vports.novo(vp->canvas);
  }
 }
 NoDados=0;
 owner=jfb;
 own=0;
 flag=OBJVISIVEL;
 lstextra=0;
}

void Objeto::add_triangle_face(Objeto *dest,Vetor *p1,Vetor *p2,Vetor *p3){
 static long v1,v2,v3,a1,a2,a3;
 char f1,f2,f3;
 if((v1=ListaVerticesM::jaexisteverts(dest,p1))==-1){
  dest->lstvert.novo(p1);
  v1=dest->lstvert.quant-1;
 }
 if((v2=ListaVerticesM::jaexisteverts(dest,p2))==-1){
  dest->lstvert.novo(p2);
  v2=dest->lstvert.quant-1;
 }
 if((v3=ListaVerticesM::jaexisteverts(dest,p3))==-1){
  dest->lstvert.novo(p3);
  v3=dest->lstvert.quant-1;
 }

 if(!(f1=ListaArestasM::jaexistearesta(dest,v1,v2,&a1))){
  dest->lstarest.novo(&dest->lstvert,v1,v2);
  a1=dest->lstarest.quant-1;
  f1=POSITIVA;
 }
 if(!(f2=ListaArestasM::jaexistearesta(dest,v2,v3,&a2))){
  dest->lstarest.novo(&dest->lstvert,v2,v3);
  a2=dest->lstarest.quant-1;
  f2=POSITIVA;
 }
 if(!(f3=ListaArestasM::jaexistearesta(dest,v3,v1,&a3))){
  dest->lstarest.novo(&dest->lstvert,v3,v1);
  a3=dest->lstarest.quant-1;
  f3=POSITIVA;
 }

 dest->lstfc.nova(dest);
 dest->lstfc.last->fc->arestas.nova(&dest->lstfc,a1,dest->lstfc.quant-1,f1);
 dest->lstfc.last->fc->arestas.nova(&dest->lstfc,a2,dest->lstfc.quant-1,f2);
 dest->lstfc.last->fc->arestas.nova(&dest->lstfc,a3,dest->lstfc.quant-1,f3);
}

void Objeto::limpa(Objeto *obj){
 obj->lstfc.Limpa();
 obj->lstarest.Limpa();
 obj->lstvert.Limpa();
 if(obj->lstextra)
  obj->lstextra->Limpa();
}


void Objeto::IniciaNormaisObjeto(Objeto *obj){
 ptrFace *fc;
 long ifc,iar;
 ptrArestaM *ar;
 for(iar=0,ar=obj->lstarest.items;ar&&iar<obj->lstarest.quant;iar++,ar++){
  Vetor::midpoint(&(obj->lstvert.items+ar->arm->p1)->vtm->pt,&(obj->lstvert.items+ar->arm->p2)->vtm->pt,&ar->arm->centro);
 }
 for(ifc=0,fc=obj->lstfc.items;ifc<obj->lstfc.quant;ifc++,fc++){
  ListaFacesM::updateparamsface(obj,fc->fc);
 }
 if(obj->lstfc.quant>0)
  CalculaMedNormaisObjeto(obj);
}

void VerticeM::calcmednormal(Objeto *obj,long indvt){
 Inteiro *ifc;
 Face *fc;
 ptrVerticeM *vert;
 Vetor normal;
 ListaInteiros lstfaces;
 vert=obj->lstvert.items+indvt;
 ListaVerticesM::get_adjacent_facesEX(obj,indvt,&lstfaces);
 Vetor_vetornulo(&normal);
 Vetor_vetornulo(&vert->vtm->mednorm);
 for(ifc=lstfaces.prm;ifc;ifc=ifc->prx){
  fc=(obj->lstfc.items+ifc->val)->fc;
  Vetor_soma(normal,fc->normal,normal);
 }
 if(lstfaces.quant>0)
  Vetor_div(normal,(NDOUBLE)lstfaces.quant,vert->vtm->mednorm);
 Vetor::normaliza(&vert->vtm->mednorm);
}

void Objeto::CalculaMedNormaisObjeto(Objeto *obj){
 long ii;
 for(ii=0;ii<obj->lstvert.quant;ii++){
  VerticeM::calcmednormal(obj,ii);
 }
}

void Objeto::faces_transform(Objeto *obj,lst_Transform *lsttransf,bool inversa){
 ptrFace *fc;
 ptrVerticeM *vert;
 if(!lsttransf)return;
 long ii;
 if(inversa){
  for(ii=0,fc=obj->lstfc.items;ii<obj->lstfc.quant;ii++,fc++){
   for(Transformacao *transf=lsttransf->getlast();transf;transf=lsttransf->getprior(transf)){
    transf->FazTransformacaoInversa(&fc->fc->centroide);
    fc->fc->normal.flag=VETOR;
    transf->FazTransformacaoInversa(&fc->fc->normal);
   }
  }
  for(ii=0,vert=obj->lstvert.items;ii<obj->lstvert.quant;ii++,vert++){
   for(Transformacao *transf=lsttransf->getlast();transf;transf=lsttransf->getprior(transf)){
    vert->vtm->mednorm.flag=VETOR;
    transf->FazTransformacaoInversa(&vert->vtm->mednorm);
   }
  }
 }
 else{
  for(ii=0,fc=obj->lstfc.items;ii<obj->lstfc.quant;ii++,fc++){
   for(Transformacao *transf=lsttransf->getfirst();transf;transf=lsttransf->getnext(transf)){
    transf->FazTransformacoes(&fc->fc->centroide);
    fc->fc->normal.flag=VETOR;
    transf->FazTransformacoes(&fc->fc->normal);
   }
  }
  for(ii=0,vert=obj->lstvert.items;ii<obj->lstvert.quant;ii++,vert++){
   for(Transformacao *transf=lsttransf->getfirst();transf;transf=lsttransf->getnext(transf)){
    vert->vtm->mednorm.flag=VETOR;
    transf->FazTransformacoes(&vert->vtm->mednorm);
   }
  }
 }
}

void Objeto::copia(ObjetoLeve *objOrigem){
 lstvert.copia(&objOrigem->lstvert);
 lstarest.copia(&lstvert,&objOrigem->lstarest,&objOrigem->lstvert);
}

void Objeto::appendVAF(Objeto *objOrigem){
 long vtiniind=lstvert.quant,ariniind=lstarest.quant,fcini=lstfc.quant;
 lstvert.copia(&objOrigem->lstvert,false);//,false,0,fcini);
 lstarest.copia(&lstvert,&objOrigem->lstarest,vtiniind,fcini,false);
 lstfc.copia(this,&objOrigem->lstfc,ariniind);
}

void Objeto::copia(Objeto *objOrigem,bool store_in_mat_list){
 id=objOrigem->id;
 propriedades = PropObj::Aloca(objOrigem->NoDados,this);
 NoDados=objOrigem->NoDados;
 flag=objOrigem->flag;
 if(!store_in_mat_list)
  __UNSETBIT(flag,OBJCANPUTINMATLIST);
 __UNSETBIT(flag,OBJSELECIONADO);
 for(int i=0;i<objOrigem->NoDados;i++)
  PropObj::copia(propriedades+i,objOrigem->propriedades+i,store_in_mat_list?this:0); 

 if(objOrigem->lstextra){
  lstextra=new ListaVerticesM;
  lstextra->copia(objOrigem->lstextra);
 }

 cCor=objOrigem->cCor;
 borda=objOrigem->borda;
 own=objOrigem->own;
 owner=objOrigem->owner;

 appendVAF(objOrigem);


 Objeto::IniciaNormaisObjeto(this);
}

Objeto::~Objeto(){
 if(NoDados>0){
  delete []propriedades;
 }
 if(lstextra)
  delete lstextra;

}

DefinicaoObjetos::~DefinicaoObjetos(){
 if(descpropriedades){
  for(int i=0;i<NumTiposDeObjetos;i++){
   if(descpropriedades[i])
    delete []descpropriedades[i];
  }
  delete []descpropriedades;
  descpropriedades=NULL;
 }
}

LinhaProp::LinhaProp(){
 caption=0;
 flag=PROPVISIVEL;
}

LinhaProp::~LinhaProp(){
 if(caption)
  free(caption);
}

void LinhaProp::aloca(const char *cap,TipoPropObj tp,char flg,int tprecriacao,double _minval,double _maxval){
 ALLOC(caption,char,n_strlen(cap)+1);
 n_strcpy(caption,cap);
 tipo=tp;
 flag=flg;
 tipo_recriacao=tprecriacao;
 minval=_minval;
 maxval=_maxval;
}

void DefinicaoObjetos::iniciadesc2D(int indobjeto){
 descpropriedades[indobjeto][0].aloca(STR_material,PROPMATERIAL);
 descpropriedades[indobjeto][1].aloca(STR_name,PROPSTRING);
 descpropriedades[indobjeto][2].aloca(STR_color,PROPCOROBJ);
 descpropriedades[indobjeto][3].aloca(STR_axis,PROPEIXOSCOORD,PROPOCULTA);
 descpropriedades[indobjeto][4].aloca(STR_linetype,PROPTIPOLIN);
 descpropriedades[indobjeto][5].aloca(STR_linescale,PROPREAL,PROPVISIVEL,0,0,__MAXINT);
 descpropriedades[indobjeto][6].aloca(STR_linewidth,PROPLINEWIDTH);
 descpropriedades[indobjeto][7].aloca(STR_renderable,PROPBOOL);

}

void DefinicaoObjetos::iniciadesc3D(int indobjeto){
 iniciadesc2D(indobjeto);
}

void DefinicaoObjetos::invoca_atualizacao(Objeto *obj){
 if(this->DefObjeto[obj->id].funcatualiza)
  this->DefObjeto[obj->id].funcatualiza(obj,0);
}

void DefinicaoObjetos::invoca_transf(Objeto *obj,lst_Transform *lsttransf,bool inversa){
 if(this->DefObjeto[obj->id].functransf)
  this->DefObjeto[obj->id].functransf(obj,lsttransf,inversa);
}

void DefinicaoObjetos::invoca_rend(Objeto *obj,NZbuf *zbuf,lst_Transform *lsttransf,int indvis,bool recalccoords){
 Canvas* canvas=zbuf->owner;
 if(recalccoords)
  canvas->estado|=RECALCULACOORDS;
 DefinicaoObjeto* defobj=janMae->ExDefinicaoObjetos->DefObjeto+obj->id;
 if(defobj->funcrend[indvis])
  defobj->funcrend[indvis](obj,zbuf,lsttransf);
 if(recalccoords)
  canvas->estado^=RECALCULACOORDS;
}

void DefinicaoObjetos::iniciadescpropriedades(){
 descpropriedades[PARABOLOIDE]=new LinhaProp[DefObjeto[PARABOLOIDE].NumDados];
 iniciadesc3D(PARABOLOIDE);
 descpropriedades[PARABOLOIDE][NUMPARAMS3D].aloca(STR_npts_M_direction,PROPINTEIRO);
 descpropriedades[PARABOLOIDE][NUMPARAMS3D+1].aloca(STR_npts_N_direction,PROPINTEIRO);
 descpropriedades[PARABOLOIDE][NUMPARAMS3D+2].aloca(STR_height,PROPREAL);
 descpropriedades[CORTE]=new LinhaProp[DefObjeto[CORTE].NumDados];
 iniciadesc2D(CORTE);
}

void DefinicaoObjeto::init(DefinicaoObjeto *dest){
 n_strcpy(dest->nome,"");
 dest->funcatualiza=0;
 dest->funcprecision=0;
 dest->funcchangepts=FuncChangePts::padrao;
 dest->funcapara=0;
 dest->funcexplode=0;
 dest->funccria=0;
 dest->funcreseta=0;
 dest->funcinitprops=0;
 dest->funcsel=FuncSelecObj::padrao;
 dest->funcinterceptrayobj=clFuncInterceptRayObj::padrao;
 dest->funcinterceptrayobjV=clFuncInterceptRayObjV::padrao;
 dest->funcborda=FuncBordasObj::padrao;
 dest->funcselpt=FuncSelPontoObj::padrao;
 dest->functransf=FuncTranformaObjeto::padrao;
 dest->funcgrava=FuncGravaObjeto::padrao;
 dest->funccarrega=FuncCarregaObjeto::padrao;
 dest->funccollisiontest=0;
 dest->NumDados=0;
 dest->instancias=0;

 for(int i=0;i<NUMRENDERTIPOS;i++)dest->funcrend[i]=FuncDesenhoObj::padrao;
}

DefinicaoObjeto::DefinicaoObjeto(){
 init(this);
}

class dlgChooseLang:public wxDialog{
public:
 dlgChooseLang(wxWindow *parent,int index);
 void on_btok_clicked();
private:
 wxComboBox *combobox;
 wxButton *btn_close;

 void OnDestroy( wxWindowDestroyEvent& event );
 DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(dlgChooseLang,wxDialog)
 EVT_WINDOW_DESTROY( dlgChooseLang::OnDestroy )
 END_EVENT_TABLE()

 //EVT_BUTTON(wxID_OK,dlgChooseLang::on_btok_clicked)
 void dlgChooseLang::OnDestroy( wxWindowDestroyEvent& event ){
  event.Skip();
}

void dlgChooseLang::on_btok_clicked(){
 JanelaMae::opcoes->mn_language=combobox->GetCurrentSelection();
 JanelaMae::opcoes->flag|=OPINIALTEROU;
 if(janMae)janMae->SetFocus();
}

dlgChooseLang::dlgChooseLang(wxWindow *parent,int index):wxDialog(parent, wxID_ANY, _T("Choose idiom"), wxDefaultPosition, wxSize(200, 100)){
 wxArrayString cbitems(0);
 for(int i=0;i<NUM_LANG_SUPPORT;i++)cbitems.Add(wxString::FromUTF8(DefinicaoObjetos::lang_descr[i]));
 wxBoxSizer *szV=new wxBoxSizer(wxVERTICAL);
 combobox = new wxComboBox(this, wxID_ANY, wxT(""), wxPoint(-1, -1), wxSize(-1, -1), cbitems);
 combobox->SetSelection(index);
 wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL );
 szV->Add(combobox,0,wxALL,10);
 btn_close=new wxButton(this, wxID_OK, wxT("Ok"), wxPoint(-1, -1), wxSize(-1, -1));
 button_sizer->Add(btn_close,0,wxALL,10);
 szV->Add(button_sizer,0,wxALIGN_CENTER );
 SetIcon(wxNullIcon);
 this->SetSizer(szV);
 szV->SetSizeHints( this );
 //SetSize(8,8,320,143);
 Center();

}

char *_str_list[quant_strings];
LangInfo DefinicaoObjetos::mn_lang_files[NUM_LANG_SUPPORT]={
 {"mn_eng.lan",0},
 {"mn_ptbr.lan",0},
 {"mn_spain.lan",0},
 {"mn_japan.lan",0}
};
LangInfo DefinicaoObjetos::lang_files[NUM_LANG_SUPPORT]={
 {"str_eng.lan",0},
 {"str_ptbr.lan",0},
 {"str_spain.lan",0},
 {"str_japan.lan",0}
};

char DefinicaoObjetos::lang_descr[NUM_LANG_SUPPORT][64];

FVOID DefinicaoObjetos::init_lang_desc(){
 static bool inited=false;
 if(!inited){
  char path[_MAX_PATH];
  n_sprintf(path,"%s%slang%s%s",JanelaMae::opcoes->diretorio_common,ARQSEPC,ARQSEPC,"lang_support.lan");
  FILE *fstring=fopen(path,"rt");
  int index_string=0;
  bool res;
  char tmpbuf[256];
  if(fstring==NULL){
   FuncUtils::erroMsg("file lang_support.lan is corrupted or dont exists, please reinstall Atena3D!");
   exit(0);
  }
  do{
   res=DXF_RW::lelinha(fstring,tmpbuf);
   if(res){
    strcpy(*(lang_descr+index_string),tmpbuf);
    index_string++;
   }
  }while (res);
  fclose(fstring);
  inited=true;
 }
}

FVOID DefinicaoObjetos::init(bool reinit){
 init_lang_desc();
 dlgChooseLang *dlglang;
 if(reinit){
  do{
   dlglang=new dlgChooseLang(janMae,JanelaMae::opcoes->mn_language);
   if(dlglang->ShowModal()==wxID_OK){
    dlglang->on_btok_clicked();
   }
   delete  dlglang;
  }while(JanelaMae::opcoes->mn_language==LAN_UNDEFINED);
 }
 else{
  if(JanelaMae::opcoes->mn_language==LAN_UNDEFINED){
#ifdef WIN32
   DWORD dwDisp;
   wchar_t *diretprog=FuncUtils::_UTF8ToUTF16(JanelaMae::opcoes->diretorio_prog);
   TCHAR dwData[_MAX_PATH];
   HKEY hk,hk2;
   RegCreateKeyEx(HKEY_CLASSES_ROOT, L".dac",0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hk,&dwDisp);
   swprintf(dwData,L"atena3d_drawing");
   RegSetValueEx(hk,NULL,0,REG_SZ,(PBYTE)&dwData,wcslen(dwData)*sizeof(wchar_t));
   RegCreateKeyEx(HKEY_CLASSES_ROOT, dwData,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hk,&dwDisp);
   swprintf(dwData,L"%s,1",JanelaMae::opcoes->progname);
   RegCreateKeyEx(hk, L"DefaultIcon",0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hk2,&dwDisp);
   RegSetValueEx(hk2,NULL,0,REG_SZ,(PBYTE)&dwData,wcslen(dwData)*sizeof(wchar_t));
   RegCreateKeyEx(hk, L"shell",0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hk,&dwDisp);
   RegCreateKeyEx(hk, L"open",0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hk,&dwDisp);
   RegCreateKeyEx(hk, L"command",0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hk,&dwDisp);
   swprintf(dwData,L"%s %%1",JanelaMae::opcoes->progname);
   RegSetValueEx(hk,NULL,0,REG_SZ,(PBYTE)&dwData,wcslen(dwData)*sizeof(wchar_t));
   _UTF16ToUTF8_clean(diretprog);
#endif
  }
  while(JanelaMae::opcoes->mn_language==LAN_UNDEFINED){
   dlglang=new dlgChooseLang(0,JanelaMae::opcoes->mn_language);
   if(dlglang->ShowModal()==wxID_OK){
    dlglang->on_btok_clicked();
   }
   delete  dlglang;
  }

  wxBitmap bitmap;
  //if (bitmap.LoadFile(wxString::Format(_("%s%s_splash.png"),wxString::FromUTF8(JanelaMae::opcoes->diretorio_common),ARQSEPC), wxBITMAP_TYPE_PNG)){
  // wxSplashScreen* splash = new wxSplashScreen(bitmap,wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_TIMEOUT,4000, NULL,-1, wxDefaultPosition,
  //  wxDefaultSize,wxSIMPLE_BORDER|wxSTAY_ON_TOP);
  //}
  wxYield();
 }
 char path[_MAX_PATH];
 n_sprintf(path,"%s%slang%s%s",JanelaMae::opcoes->diretorio_common,ARQSEPC,ARQSEPC,lang_files[JanelaMae::opcoes->mn_language].file);
 FILE *fstring=fopen(path,"rt");
 int index_string=0;
 bool res;
 char tmpbuf[256];
 do{
  res=DXF_RW::lelinha(fstring,tmpbuf);
  if(res){
   if(reinit) delete []*(_str_list+index_string);
   *(_str_list+index_string)=new char[strlen(tmpbuf)+1];
   strcpy(*(_str_list+index_string),tmpbuf);
   index_string++;
  }
 }while (res);
 fclose(fstring);
 if(reinit){
  for(int i=0;i<NumTiposDeObjetos;i++){
   delete []descpropriedades[i];
  }
  delete []descpropriedades;
  janMae->lincom->cmds.limpa();
  janMae->lincom->cmds.inicmds();
 }
 descpropriedades=new LinhaProp*[NumTiposDeObjetos];
 init_arc();
 init_circle();
 init_cline();
 init_chart();
 init_cylinder();
 init_cone();
 init_cota();
 init_drawingboard();
 init_viewport();
 init_ellipse();
 init_face3D();
 init_fraclin();
 init_hatch();
 init_image();
 init_insert();
 init_line();
 init_malha3D();
 init_lightmesh();
 init_mtext();
 init_objCPG();
 init_objCSG();
 init_paralelepipedo();
 init_wedge();
 init_pline();
 init_point();
 init_polygon();
 init_parabola();
 init_ray();
 init_rectangle();
 init_sphere();
 init_ellipsoid();
 init_hyperboloid();
 init_solid2D();
 init_spline();
 init_surface_spline();
 init_terrain();
 init_text();
 init_text3D();
 init_attdef();
 init_attrib();
 init_torus();
 init_surface_subdivision();
 init_surf_extrude();
 init_COORDSYSAUX();

 n_strcpy(DefObjeto[PARABOLOIDE].nome,"paraboloide");
 DefObjeto[PARABOLOIDE].NumDados=NUMPARAMS3D+3;
 DefObjeto[PARABOLOIDE].funcrend[VISLINOCULTAS]=FuncDesenhoObj::padraoLO;
 DefObjeto[PARABOLOIDE].funcrend[VISCONSTANTE]=FuncDesenhoObj::padraoCO;
 DefObjeto[PARABOLOIDE].funcrend[VISGOURAUD]=FuncDesenhoObj::padraoCI;
 DefObjeto[PARABOLOIDE].funcrend[VISPHONG]=FuncDesenhoObj::padraoNI;
 DefObjeto[PARABOLOIDE].funcrend[VISRAYTRACE]=FuncDesenhoObj::padraoRC;


 n_strcpy(DefObjeto[CORTE].nome,"corte");
 DefObjeto[CORTE].NumDados=NUMPARAMS2D;
 DefObjeto[CORTE].funcrend[VISQUADROARAME]=FuncDesenhoObj::corte;
 DefObjeto[CORTE].funcrend[VISLINOCULTAS]=FuncDesenhoObj::corte;
 DefObjeto[CORTE].funcrend[VISCONSTANTE]=FuncDesenhoObj::corte;
 DefObjeto[CORTE].funcrend[VISGOURAUD]=FuncDesenhoObj::corte;
 DefObjeto[CORTE].funcrend[VISPHONG]=FuncDesenhoObj::corte;
 DefObjeto[CORTE].funcsel=FuncSelecObj::corte;
 DefObjeto[CORTE].funcborda=FuncBordasObj::corte;

 iniciadescpropriedades();
}

DefinicaoObjetos::DefinicaoObjetos(){
 init();
}


void ListaObjetos::novo(JanelaFilhaBase *jfb,atnObjectType tipo){
 novoptr(jfb);
 atual->id=tipo;
}

void ListaObjetos::novo(JanelaFilhaBase *jfb,Objeto *obj){
 novoptr(jfb);
 atual->id=obj->id;
 atual->propriedades= PropObj::Aloca(obj->NoDados,atual);
 atual->NoDados=obj->NoDados;
 atual->lstvert.transmite(&obj->lstvert);
 atual->lstarest.transmite(&obj->lstarest);
 atual->lstfc.transmite(&obj->lstfc);
}

void ListaObjetos::Limpa(short SoObjeto){
 Objeto *temp;
 atual=prm;
 while(atual) {
  temp=atual->prx;
  if(SoObjeto){
   atual->lstvert.items=NULL;
   atual->lstarest.items=NULL;
   atual->lstfc.items=NULL;
  }
  delete atual;
  atual=temp;
 }
 quant=0;
 prm=ult=atual=NULL;
}

void ListaObjetos::ini_name_and_color(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb){
 static char nome[256];
 DefinicaoObjeto* defobj=janMae->ExDefinicaoObjetos->DefObjeto+obj->id;
 n_sprintf(nome,"%s_%d",defobj->nome,1+defobj->instancias++);
 if(jfb)
  PropObj::atribui(&obj->MATERIAL,jfb->lstmateriais.highlander,putinmatlist?obj:0);
 PropObj::atribui(&obj->NOME,nome);
 PropObj::atribui(&obj->COR,JanelaMae::opcoes->paleta.highlander->val);
}

void  ListaObjetos::IniciaDadosPadraoA(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb){
 ini_name_and_color(obj,putinmatlist,jfb);
 if(jfb)
  PropObj::atribui(&obj->TIPOLINHA,jfb->lstlinestilos.highlander);
 PropObj::atribui(&obj->ESCALALINHA,1.0);
 PropObj::atribui(&obj->LARGURALINHA,JanelaMae::opcoes->linewidths.highlander->val);
 PropObj::atribui(&obj->RENDERIZAVEL,(int)1);

}

void  ListaObjetos::CopiaDadosPadrao2D(Objeto *org,Objeto *dest,bool putinmatlist){
 PropObj::atribui(&dest->NOME,PropObj_getstring(&org->NOME));
 PropObj::atribui(&dest->COR,PropObj_getuint(&org->COR));
 PropObj::atribui(&dest->TIPOLINHA,(LinEstilo*)org->TIPOLINHA.valor);
 PropObj::atribui(&dest->ESCALALINHA,PropObj_getreal(&org->ESCALALINHA));
 PropObj::atribui(&dest->LARGURALINHA,PropObj_getreal(&org->LARGURALINHA));
 PropObj::atribui(&dest->RENDERIZAVEL,PropObj_getint(&org->RENDERIZAVEL));
 PropObj::atribui(&dest->MATERIAL,(Material*)org->MATERIAL.valor,putinmatlist?dest:0);
}

void  ListaObjetos::IniciaDadosPadrao3D(Objeto *obj){
 corparavetor(obj->cCor,JanelaMae::opcoes->paleta.getcolor());
 Vetor v1=Vetor(1,0,0),v2=Vetor(0,1,0),v3=Vetor(0,0,1);
 PropObj::atribuieixo(&obj->EIXOS,v1,v2,v3);
}

void ListaObjetos::alocapropriedades(Objeto *obj){
 DefinicaoObjeto* defobj=janMae->ExDefinicaoObjetos->DefObjeto+obj->id;
 obj->NoDados=(short)defobj->NumDados;
 obj->propriedades= PropObj::Aloca(defobj->NumDados,obj);

}

void  ListaObjetos::iniciapropriedades(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb){
 DefinicaoObjeto* defobj=janMae->ExDefinicaoObjetos->DefObjeto+obj->id;
 if(putinmatlist)
  obj->flag|=OBJCANPUTINMATLIST;
 else
  __UNSETBIT(obj->flag,OBJCANPUTINMATLIST);
 if(defobj->funcinitprops)
  defobj->funcinitprops(obj,putinmatlist,jfb);
}

void ListaObjetos::changeType(Objeto *obj,atnObjectType newtype,JanelaFilhaBase* jfb){
 if(obj->id!=newtype){
  obj->id=newtype;
  ((Material*)obj->MATERIAL.valor)->objs.apaga((void*)obj);
  delete[]obj->propriedades;
  ListaObjetos::alocaeiniciapropriedades(obj,true,jfb);
 }
}

void  ListaObjetos::alocaeiniciapropriedades(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb){
 alocapropriedades(obj);
 iniciapropriedades(obj, putinmatlist,jfb);
}

bool  ListaObjetos::alocaeiniciaobjeto(Objeto **obj,atnObjectType tipo,bool putinmatlist,JanelaFilhaBase* jfb){
 *obj=new Objeto(jfb);
 if(obj!=NULL){
  (*obj)->id=tipo;
  ListaObjetos::alocaeiniciapropriedades(*obj, putinmatlist,jfb);
  return true;
 }
 return false;
}

void ListaObjetos::updateTopologyInfo(Objeto *obj){
 ptrVerticeM *pvt,*v1,*v2;
 VerticeM *vt;
 ptrArestaFace *parfc;
 ArestaFace *_arfc;
 ptrFace *pfc;
 ptrArestaM *par;
 ArestaM *ar;
 long i,j;
 for(i=0,pvt=obj->lstvert.items;i<obj->lstvert.quant;i++,pvt++){
  vt=pvt->vtm;  
  vt->firstar.set(-1);
  vt->edges.limpa();
 }
 
 for(i=0,par=obj->lstarest.items;i<obj->lstarest.quant;i++,par++){
  ar=par->arm;  
  ar->vizinfo.Set(-1,-1,-1,-1);
  ar->aux.Limpa();
  v1=obj->lstvert.items+ar->p1;
  v2=obj->lstvert.items+ar->p2;
  if(ar->p1!=ar->p2){
   v1->vtm->edges.novo(par->index);
   v1->vtm->firstar.ar=par->index;
   v2->vtm->edges.novo(par->index);
   v2->vtm->firstar.ar=par->index;
  }
 }
 
 for(i=0,pfc=obj->lstfc.items;i<obj->lstfc.quant;i++,pfc++){
  for(j=0,parfc=pfc->fc->arestas.items;j<pfc->fc->arestas.quant;j++,parfc++){
   _arfc=parfc->loop;
   ptrArestaM *par=obj->lstarest.items+_arfc->a1;
   ArestaM *ar=par->arm;
   long vtest1,vtest2;
   ptrVerticeM *_v2;
   getptsfromarfc(vtest1,vtest2,_arfc,ar);
   if(vtest1!=vtest2){
    _v2=obj->lstvert.items+vtest1;
    if(_v2->vtm->firstar.ar==-1)
     _v2->vtm->firstar.set(par->index);
    if(ar->vizinfo.fe==-1&&vtest1==ar->p1){
     ar->vizinfo.fe=i;
     ar->vizinfo.arfce=parfc->index;
    }
    if(ar->vizinfo.fd==-1&&vtest1==ar->p2){
     ar->vizinfo.fd=i;
     ar->vizinfo.arfcd=parfc->index;
    }
   }
  }
 }
}

void  ListaObjetos::alocaeiniciaobjeto(ObjetoLeve **obj,atnObjectType tipo,bool putinmatlist,JanelaFilhaBase* jfb){
 *obj=new ObjetoLeve;
}

void ListaObjetos::ad_linha(ListaObjetos *dest,Vetor *p1,Vetor *p2,bool putinmatlist,JanelaFilhaBase *jfb){
 dest->novo(jfb,LINHA);
 ListaObjetos::alocaeiniciapropriedades(dest->ult,putinmatlist,jfb);
 dest->ult->lstvert.items->vtm->pt=*p1;
 (dest->ult->lstvert.items+1)->vtm->pt=*p2;
 dest->ult->lstarest.novo(&dest->ult->lstvert,0,1);
 if(dest->quant==1&&jfb)
  RetanguloBorda::AtualizaBordasTodos(jfb);
 else
  janMae->ExDefinicaoObjetos->DefObjeto[dest->ult->id].funcborda(dest->ult,0);
}

/*void ListaObjetos::ad_paralelepipedo(ListaObjetos *dest,Vetor *pc,double width,double height,double depth,bool putinmatlist,JanelaFilhaBase *jfb){
dest->novo(jfb,PARALELEPIPEDO);
ListaObjetos::alocaeiniciapropriedades(dest->ult,putinmatlist,jfb);
PropObj::atribuipt(&dest->ult->PARA_Centro,pc->x,pc->y,pc->z);
PropObj::atribui(&dest->ult->PARA_Larg,(NDOUBLE)width);
PropObj::atribui(&dest->ult->PARA_Prof,(NDOUBLE)depth);
PropObj::atribui(&dest->ult->PARA_Altura,(NDOUBLE)height);
if(janMae->ExDefinicaoObjetos->DefObjeto[dest->ult->id].funccria)
janMae->ExDefinicaoObjetos->DefObjeto[dest->ult->id].funccria(jfb,dest->ult,0);
dest->ult->lstvert.items->vtm->pt=*p1;
(dest->ult->lstvert.items+1)->pt=*p2;
dest->ult->lstarest.novo(&dest->ult->lstvert,0,1);
if(dest->quant==1&&jfb)
RetanguloBorda::AtualizaBordasTodos(jfb);
else
janMae->ExDefinicaoObjetos->DefObjeto[dest->ult->id].funcborda(dest->ult,0);
}*/

void ListaObjetos::ad_solido2d(ListaObjetos *dest,Vetor *pts,int npts,bool putinmatlist,JanelaFilhaBase *jfb){
 dest->novo(jfb,SOLIDO2D);
 ListaObjetos::alocaeiniciapropriedades(dest->ult,putinmatlist,jfb);
 Objeto::limpa(dest->ult);
 for(int i=0;i<npts;i++){
  dest->ult->lstvert.novo(pts+i);
 }
 if(dest->quant==1&&jfb)
  RetanguloBorda::AtualizaBordasTodos(jfb);
 else
  janMae->ExDefinicaoObjetos->DefObjeto[dest->ult->id].funcborda(dest->ult,0);
}

void ListaObjetos::ad_arc(ListaObjetos *dest,Vetor *pini,Vetor *pfim,Vetor *normal,Vetor *centro,double raio,bool putinmatlist,JanelaFilhaBase *jfb){
 dest->novo(jfb,ARCO);
 Vetor x,y;
 ListaObjetos::alocaeiniciapropriedades(dest->ult,putinmatlist,jfb);
 if(raio>=0.0){
  *ARC_PINI(dest->ult)=*pini;
  *ARC_PEND(dest->ult)=*pfim;
  Vetor_sub(*pfim,*pini,x);
  Vetor::normaliza(&x);
  Vetor::pvet(normal,&x,&y);
  Vetor_mult(y,raio,*ARC_PMID(dest->ult));
  Vetor_soma(*ARC_PMID(dest->ult),*centro,*ARC_PMID(dest->ult));
 }
 else{
  *ARC_PINI(dest->ult)=*pini;
  *ARC_PMID(dest->ult)=*normal;
  *ARC_PEND(dest->ult)=*pfim;
 }
 FuncAtualizaObj::arc(dest->ult);
 if(dest->quant==1&&jfb)
  RetanguloBorda::AtualizaBordasTodos(jfb);
 else
  janMae->ExDefinicaoObjetos->DefObjeto[dest->ult->id].funcborda(dest->ult,0);
}

void ListaObjetos::ad_circle(ListaObjetos *dest,int nseg,double angini,double angfim,Vetor *normal,Vetor *centro,double raio,bool putinmatlist,JanelaFilhaBase *jfb){
 dest->novo(jfb,CIRCULO);
 ListaObjetos::alocaeiniciapropriedades(dest->ult,putinmatlist,jfb);
 dest->ult->lstvert.Limpa();
 dest->ult->lstvert.novo(centro);
 dest->ult->lstvert.novo(normal->x,normal->y,normal->z,0.0,0.0,VETOR);
 PropObj::atribui(&dest->ult->CIRC_RAIO,(NDOUBLE)raio);
 PropObj::atribui(&dest->ult->CIRC_NOCONEC,(int)nseg);
 PropObj::atribui(&dest->ult->CIRC_ANGINI,(NDOUBLE)angini);
 PropObj::atribui(&dest->ult->CIRC_ANGFIM,(NDOUBLE)angfim);
 if(dest->quant==1&&jfb)
  RetanguloBorda::AtualizaBordasTodos(jfb);
 else
  janMae->ExDefinicaoObjetos->DefObjeto[dest->ult->id].funcborda(dest->ult,0);
}

void ListaObjetos::ad_text(ListaObjetos *dest,Vetor *ptins,const char *text,Vetor *i,Vetor *j,double altura,int txt_align,int transp,bool putinmatlist,JanelaFilhaBase *jfb){
 dest->novo(jfb,TEXTO);
 ListaObjetos::alocaeiniciapropriedades(dest->ult,putinmatlist,jfb);
 Vetor k;
 Vetor::pvet(i,j,&k);
 PropObj::atribuieixo(&dest->ult->EIXOS,*i,*j,k);
 Vetor_detvalfast(&(dest->ult->lstvert.items)->vtm->pt,ptins);
 PropObj::atribui(&dest->ult->TXT_ALTURA,(NDOUBLE)altura);
 PropObj::atribui(&dest->ult->TXT_TEXTO,text);
 PropObj::atribui(&dest->ult->TXT_ALINHAMENTO,(int)txt_align);
 PropObj::atribui(&dest->ult->TXT_TRANSP,(int)transp);
 if(dest->quant==1&&jfb)
  RetanguloBorda::AtualizaBordasTodos(jfb);
 else
  janMae->ExDefinicaoObjetos->DefObjeto[dest->ult->id].funcborda(dest->ult,0);
}

void ListaObjetos::ad_attrib(ListaObjetos *dest,Vetor *ptins,char *attdef,char *text,int flags,double alttext,
 Vetor *i,Vetor *j,int txt_align,bool putinmatlist,JanelaFilhaBase *jfb){
  dest->novo(jfb,_ATTRIB);
  ListaObjetos::alocaeiniciapropriedades(dest->ult,putinmatlist,jfb);
  if(ptins)
   Vetor_detvalfast(&(dest->ult->lstvert.items)->vtm->pt,ptins);
  if(i)
   Vetor_detvalfast(&(dest->ult->lstvert.items+1)->vtm->pt,i);
  if(j)
   Vetor_detvalfast(&(dest->ult->lstvert.items+2)->vtm->pt,j);

  PropObj::atribui(&dest->ult->ATTRIB_ALTURA,(NDOUBLE)alttext);
  PropObj::atribui(&dest->ult->ATTRIB_FLAGS,flags);
  PropObj::atribui(&dest->ult->ATTRIB_TEXT,text);
  PropObj::atribui(&dest->ult->ATTRIB_ALINHAMENTO,(int)txt_align);
  PropObj::atribui(&dest->ult->ATTRIB_ATTDEF,attdef);
  if(dest->quant==1&&jfb)
   RetanguloBorda::AtualizaBordasTodos(jfb);
  else
   janMae->ExDefinicaoObjetos->DefObjeto[dest->ult->id].funcborda(dest->ult,0);
}

void ListaObjetos::ad_attdef(ListaObjetos *dest,Vetor *ptins,char *label,char *text,int flags,double alttext,
 Vetor *i,Vetor *j,int txt_align,bool putinmatlist,JanelaFilhaBase *jfb){
  dest->novo(jfb,_ATTDEF);
  ListaObjetos::alocaeiniciapropriedades(dest->ult,putinmatlist,jfb);
  if(ptins)
   Vetor_detvalfast(&(dest->ult->lstvert.items)->vtm->pt,ptins);
  if(i)
   Vetor_detvalfast(&(dest->ult->lstvert.items+1)->vtm->pt,i);
  if(j)
   Vetor_detvalfast(&(dest->ult->lstvert.items+2)->vtm->pt,j);

  PropObj::atribui(&dest->ult->ATTDEF_ALTURA,(NDOUBLE)alttext);
  PropObj::atribui(&dest->ult->ATTDEF_FLAGS,flags);
  PropObj::atribui(&dest->ult->ATTDEF_TEXT,text);
  PropObj::atribui(&dest->ult->ATTDEF_ALINHAMENTO,(int)txt_align);
  PropObj::atribui(&dest->ult->ATTDEF_LABEL,label);
  if(dest->quant==1&&jfb)
   RetanguloBorda::AtualizaBordasTodos(jfb);
  else
   janMae->ExDefinicaoObjetos->DefObjeto[dest->ult->id].funcborda(dest->ult,0);
}

void  ListaObjetos::IniciaDadosObjetos(ListaObjetos *lstobjs,JanelaFilhaBase *jfb,Objeto **obj,bool atualizavista,bool addtoundo){
 JanelaMae::registraalteracoespranchas(jfb);
 (*obj)->prx=NULL;
 long quant=0;
 if(lstobjs){
  if(lstobjs->prm) {
   (*obj)->ant=lstobjs->ult;
   lstobjs->ult->prx=(*obj);
  }
  else {
   (*obj)->ant=NULL;
   lstobjs->prm=*obj;
  }
  lstobjs->ult=*obj;
  lstobjs->atual=*obj;
  lstobjs->atual->id=(*obj)->id;
  lstobjs->quant++;
  quant=lstobjs->quant;

 }
 if(quant==1)
  RetanguloBorda::AtualizaBordasTodos(jfb);
 else
  janMae->ExDefinicaoObjetos->DefObjeto[(*obj)->id].funcborda(*obj,0);
 Objeto::IniciaNormaisObjeto(*obj);
 jfb->BorderAllObjects.MR.obtmaxmin(&(*obj)->borda.MR);
 unsigned int cor=janMae->opcoes->paleta.getcolor((*obj));
 corparavetor((*obj)->cCor,cor);
 if((*obj)->flag&OBJSELECIONADO){
  jfb->objselec.apagabyobj((*obj));//evita selecionar duas vezes o mesmo objeto
  __UNSETBIT((*obj)->flag,OBJSELECIONADO);
  jfb->objselec.novo((*obj),NULL);
 }
 /*ListaPoligonos lstpol;
 lstpol.criamundo(jfb.objs);
 BSP_tree::limpa(jfb->bsptree);
 jfb->bsptree=BSP_tree::Build_BSP_Tree ( &lstpol);*/
 if(atualizavista)
  jfb->viewports.atualiza(true,true);
 if(addtoundo){
  jfb->lstundorec.novalistaundo(*obj,UNDO_INSERCAO_OBJETO,UNDO,0,true,true);
 }
 if(lstobjs)
  *obj=NULL;
}

Objeto *ListaObjetos::copia(Objeto *objOrigem,bool store_in_mat_list){
 novo(objOrigem->owner,objOrigem->id);
 ult->copia(objOrigem,store_in_mat_list);
 return ult;
}

void ListaObjetos::clone(ListaObjetos *org){
 Objeto *obj;
 for(obj=org->prm;obj;obj=obj->prx){
  copia(obj,(obj->flag&OBJCANPUTINMATLIST)!=0);
 }
}

void ObjetoLeve::copia(Poligono *pol){
 ptrVerticeM *v;
 for(int i=0;i<pol->vertices.quant;i++){
  v=&pol->vertices.items[i];
  lstvert.novo(&v->vtm->pt);
  if(i>0)
   lstarest.novo(lstvert.ult->ant,lstvert.ult);
 }
 lstarest.novo(lstvert.ult,lstvert.prm);
}

void ObjetoLeve::copia(Objeto *obj,long fc){
 ptrArestaFace *arfc;
 Face *face=(obj->lstfc.items+fc)->fc;
 ListaArestasM *lstar=&obj->lstarest;
 ListaVerticesM *plstvert=&obj->lstvert;
 ptrVerticeM *v;
 int i;
 for(i=0,arfc=face->arestas.items;i<face->arestas.quant;i++,arfc++){
  if(arfc->loop->FLAG&AREST_INVERTIDA)
   v=plstvert->items+lstar->items[arfc->loop->a1].arm->p2;
  else
   v=plstvert->items+lstar->items[arfc->loop->a1].arm->p1;
  lstvert.novo(&v->vtm->pt);
  if(i>0)
   lstarest.novo(lstvert.ult->ant,lstvert.ult);
 }
 usrdado=(void*)face;
 usrdado2=obj;
 lstarest.novo(lstvert.ult,lstvert.prm);
 borda.CalculaBordas(&lstvert,0);
} 

void ObjetoLeve::copia(ObjetoLeve *ol){
 lstvert.copia(&ol->lstvert);
 lstarest.copia(&lstvert,&ol->lstarest);
 usrdado=ol->usrdado;
 usrdado2=ol->usrdado2;
 borda=ol->borda;
 flag=ol->flag;
}

void ObjetoLeve::limpa(){
 lstarest.Limpa();
 lstvert.Limpa();
}

void ListaObjetoLeve::ad(Objeto *obj,long fc){
 novoptr();
 atual->copia(obj,fc);
}

void ListaObjetoLeve::ad(ObjetoLeve *ol)
{
 novoptr();
 atual->copia(ol);
}

void ListaObjetoLeve::criaobjeto(Objeto *obj){
 for(long i=0;i<obj->lstfc.quant;i++){
  ad(obj,i);
 }
}
