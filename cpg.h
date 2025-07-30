#ifndef cpgH
#define cpgH
#include "objeto.h"
#include "csg.h"
#include "funcglob.h"

class Cpg{
public:
 static FVOID inserealpha(Aresta *ar,NDOUBLE alpha,Vetor *p);
 static bool procarestcolineares(ObjetoLeve *a,ObjetoLeve *b,Aresta *ara,Aresta *arb,ObjetoLeve *asobrebmais,ObjetoLeve *asobrebmenos);
 static int compalphas(Ponteiro *a,Ponteiro *b);
 static FVOID copiaarestasparalstptr(ListaArestas *arestas,ListaPonteiros *lptr1,ListaPonteiros *lptr2);
 static Objeto *percorrearvore(JanelaFilhaBase *jfb,CSGNO *s);
};

template <class TO>
class Cpgfunc{
public:
 static FVOID processaalphas(Aresta *ar,TO *clipobj,Vetor *i,Vetor *j,ObjetoLeve *objdentro,ObjetoLeve *objfora);
 static FVOID geraresultado(BOOLOP op,TO *b,ObjetoLeve *asobrebmais,ObjetoLeve *asobrebmenos,ObjetoLeve *aforab,
  ObjetoLeve *adentrob,ObjetoLeve *bforaa,ObjetoLeve *bdentroa,ObjetoLeve *dest,Vetor *vi,Vetor *vj,Vetor *k);
 static FVOID adarestaparalista(TO *obj,Aresta *ar);
 static FVOID adarestapararesult(Aresta *ar,ObjetoLeve *dest);
 static FVOID advertspararesult(Vertice *v1,Vertice *v2,ObjetoLeve *dest);
 static FVOID adarestainvertpararesult(Aresta *ar,ObjetoLeve *dest);
 static FVOID jaexisteverts(TO *obj,Vertice *v1,Vertice *v2,Vertice **vres1,Vertice **vres2);
 static TO *booloppoligono(TO *a,TO *b,BOOLOP op,Face *f,JanelaFilhaBase *jfb);

};
template <class TO>
FVOID Cpgfunc<TO>::jaexisteverts(TO *obj,Vertice *v1,Vertice *v2,Vertice **vres1,Vertice **vres2){
 Vertice *vt=obj->lstvert.prm;
 *vres1=*vres2=0;
 while(vt&&((!(*vres1))||(!(*vres2)))) {
  if(__pontosiguais(vt->pt,v1->pt)&&(!(*vres1)))
   *vres1=vt;
  if(__pontosiguais(vt->pt,v2->pt)&&(!(*vres2)))
   *vres2=vt;
  vt=vt->prx;
 }
}

template <class TO>
FVOID Cpgfunc<TO>::adarestaparalista(TO *obj,Aresta *ar){
 Vertice *vres1,*vres2;
 jaexisteverts(obj,ar->p1,ar->p2,&vres1,&vres2);
 if(!vres1){
  obj->lstvert.novo(ar->p1);
  vres1=obj->lstvert.ult;
 }
 if(!vres2){
  obj->lstvert.novo(ar->p2);
  vres2=obj->lstvert.ult;
 }
 unsigned char flag=ar->FLAG;
 __UNSETBIT(flag,AREST_SELECIONADA);
 obj->lstarest.novo(vres1,vres2,flag);
}

template <class TO>
FVOID Cpgfunc<TO>::adarestapararesult(Aresta *ar,ObjetoLeve *dest){
 Vertice *vres1,*vres2;
 if(!__pontosiguais(ar->p1->pt,ar->p2->pt))
 {
  Cpgfunc<ObjetoLeve>::jaexisteverts(dest,ar->p1,ar->p2,&vres1,&vres2);
  if(!vres1)
  {
   dest->lstvert.novo(ar->p1);
   vres1=dest->lstvert.ult;
  }
  if(!vres2)
  {
   dest->lstvert.novo(ar->p2);
   vres2=dest->lstvert.ult;
  }
  unsigned char flag=ar->FLAG;
 __UNSETBIT(flag,AREST_SELECIONADA);
  dest->lstarest.novo(vres1,vres2,flag);
 }
}

template <class TO>
FVOID Cpgfunc<TO>::advertspararesult(Vertice *v1,Vertice *v2,ObjetoLeve *dest){
 Vertice *vres1,*vres2;
 if(!__pontosiguais(v1->pt,v2->pt))
 {
  Cpgfunc<ObjetoLeve>::jaexisteverts(dest,v1,v2,&vres1,&vres2);
  if(!vres1)
  {
   dest->lstvert.novo(v1);
   vres1=dest->lstvert.ult;
  }
  if(!vres2)
  {
   dest->lstvert.novo(v2);
   vres2=dest->lstvert.ult;
  }
  dest->lstarest.novo(vres1,vres2);
 }
}

template <class TO>
FVOID Cpgfunc<TO>::adarestainvertpararesult(Aresta *ar,ObjetoLeve *dest){
 Vertice *vres1,*vres2;
 if(!__pontosiguais(ar->p1->pt,ar->p2->pt))
 {
  jaexisteverts(dest,ar->p1,ar->p2,&vres1,&vres2);
  if(!vres1)
  {
   dest->lstvert.novo(ar->p1);
   vres1=dest->lstvert.ult;
  }
  if(!vres2)
  {
   dest->lstvert.novo(ar->p2);
   vres2=dest->lstvert.ult;
  }
  dest->lstarest.novo(vres2,vres1);
 }
}

template <class TO>
FVOID Cpgfunc<TO>::processaalphas(Aresta *ar,TO *clipobj,Vetor *i,Vetor *j,ObjetoLeve *objdentro,ObjetoLeve *objfora){
 static ListaVertices vertstmp;
 static ListaArestas aresttmp;
 static Vetor pm;
 PontoInterseccao *ptint,*ptintprx;
 Cpg::inserealpha(ar,1.0,&ar->p2->pt);
 ar->aux.prm=ListaPonteiros::ordena(ar->aux.prm,&ar->aux.ult,(FuncCompara*)Cpg::compalphas);
 Ponteiro *ptr=ar->aux.prm;
 while(ptr&&ptr->prx){
  ptint=(PontoInterseccao *)ptr->end;
  ptintprx= (PontoInterseccao *)ptr->prx->end;
  Vetor_soma(ptint->pt,ptintprx->pt,pm);
  Vetor_mult(pm,0.5,pm);
  vertstmp.Limpa();
  aresttmp.Limpa();
  vertstmp.novo(&ptint->pt);
  vertstmp.novo(&ptintprx->pt);
  aresttmp.novo(vertstmp.prm,vertstmp.ult);
  if(FuncoesGeometricas::pt_dentro_arestasang(&clipobj->lstarest,i,j,&pm))
   Cpgfunc<ObjetoLeve>::adarestaparalista(objdentro,aresttmp.ult);
  else
   Cpgfunc<ObjetoLeve>::adarestaparalista(objfora,aresttmp.ult);
  ptr=ptr->prx;
 }
}

template <class TO>
FVOID Cpgfunc<TO>::geraresultado(BOOLOP op,TO *b,ObjetoLeve *asobrebmais,ObjetoLeve *asobrebmenos,ObjetoLeve *aforab,
 ObjetoLeve *adentrob,ObjetoLeve *bforaa,ObjetoLeve *bdentroa,ObjetoLeve *dest,Vetor *vi,Vetor *vj,Vetor *k){
#define geraarestas(lista)\
  {ar=lista->lstarest.prm;\
  while(ar){Cpgfunc<ObjetoLeve>::adarestapararesult(ar,&prvresult);ar=ar->prx;}}
  ObjetoLeve prvresult;
  ListaPonteiros lstptr1,lstptr2;
  Aresta *ar,*ar2;
  NDOUBLE minptdist=MAXFLOAT;
  Vetor testevt;
  dest->lstarest.Limpa();
  dest->lstvert.Limpa();
  switch(op){
  case UNIAO:
   geraarestas(aforab)
    geraarestas(bforaa)
    geraarestas(asobrebmais)
    break;
  case INTERSECCAO:
   geraarestas(adentrob)
    geraarestas(bdentroa)
    geraarestas(asobrebmais)
    break;
  case DIFERENCA:
   geraarestas(aforab)
    ar=bdentroa->lstarest.ult;
   while(ar){
    Cpgfunc<ObjetoLeve>::adarestainvertpararesult(ar,&prvresult);
    ar=ar->ant;
   }
   geraarestas(asobrebmenos)
    break;
  default:break;
  }
  Cpg::copiaarestasparalstptr(&prvresult.lstarest,&lstptr1,&lstptr2);
  Ponteiro *ptr1,*ptr2;
  bool apagou;
  ptr1=lstptr1.prm;
  while(ptr1){
   apagou=false;
   for(ptr2=lstptr2.prm;ptr2;ptr2=ptr2->prx){
    if(ptr2->end==ptr1->end)
     continue;
    if(((Aresta*)ptr2->end)->p1==((Aresta*)ptr1->end)->p2){
     if(FuncoesGeometricas::arestascolineares((Aresta*)ptr1->end,(Aresta*)ptr2->end)){
      advertspararesult(((Aresta*)ptr1->end)->p1,((Aresta*)ptr2->end)->p2,dest);
      if(((Aresta*)ptr2->end)->FLAG&AREST_SELECIONADA){
       //dest->lstvert.apaga(((Aresta*)((Aresta*)ptr2->end)->aux.prm->end)->p1);
       dest->lstarest.apaga(((Aresta*)((Aresta*)ptr2->end)->aux.prm->end));
      }
      ((Aresta*)ptr1->end)->p2=((Aresta*)ptr2->end)->p2;
      lstptr1.apaga(ptr1);
      lstptr1.apaga(ptr2->end);
      ptr1=0;
      Ponteiro *ptr=lstptr2.prm;
      while(ptr){
       if(((Aresta*)ptr->end)->p1==((Aresta*)ptr2->end)->p2){
        ptr1=lstptr1.procura(ptr->end);break;
       }
       ptr=ptr->prx;
      }
      lstptr2.apaga(ptr2);
      apagou=true;
     }
     else{
      adarestapararesult(((Aresta*)ptr1->end),dest);
      ((Aresta*)ptr1->end)->FLAG|=AREST_SELECIONADA;
      ((Aresta*)ptr1->end)->aux.novo(dest->lstarest.ult);
      lstptr1.apaga(ptr1);
      ptr1=lstptr1.procura(ptr2->end);
      if(!ptr1){
       Ponteiro *ptr=lstptr1.prm;
       while(ptr){
        if(((Aresta*)ptr->end)->p1==((Aresta*)ptr2->end)->p2){
         ptr1=ptr;break;
        }
        ptr=ptr->prx;
       }
      }
      apagou=true;
     }
     break;
    }
   }
   if(!apagou)
    ptr1=ptr1->prx;
   else if(!ptr1)
    ptr1=lstptr1.prm;
  }
  if(dest&&dest->lstarest.ult)
  {
   ObjetoLeve loop,*oaptr,*obptr;
   ListaObjetoLeve lstloops;
   Aresta *ar,*prm;
   Vertice *vt;
   int numvtint;
   for(prm=ar=dest->lstarest.prm;ar;ar=ar->prx){
    loop.lstvert.novo(ar->p1);
    if(loop.lstvert.atual->ant){
     loop.lstarest.novo(loop.lstvert.atual->ant,loop.lstvert.atual);
     if(ar->ant)
      loop.lstarest.ult->aux.novo(ar->ant);
    }
    if(ar->p2==prm->p1){//fim de um loop
     loop.lstarest.novo(loop.lstvert.ult,loop.lstvert.prm);
     loop.lstarest.ult->aux.novo(ar);
     lstloops.ad(&loop);
     lstloops.ult->borda.CalculaBordas(&lstloops.ult->lstvert,0);
     loop.limpa();
     prm=ar->prx;
    }
   }
   bool reverteu;
   for(oaptr=lstloops.prm;oaptr;oaptr=oaptr->prx){
    for(obptr=lstloops.prm;obptr;obptr=obptr->prx){
     if(oaptr==obptr)continue;
     if(oaptr->borda.MR.bordainterna(&obptr->borda.MR)){
      numvtint=0;
      for(vt=obptr->lstvert.prm;vt;vt=vt->prx){
       if(FuncoesGeometricas::pt_dentro_arestasang(&oaptr->lstarest,vi,vj,&vt->pt))
        numvtint++;
      }
      if(numvtint==obptr->lstvert.quant){//loop é interior
       if((reverteu=(!ListaArestas::clockwise(&obptr->lstarest,vi,vj,k))))
        ListaArestas::revert(&obptr->lstarest);
       Aresta *arfora1=0,*arfora2=0,*ardentro1=0,*ardentro2=0;
       ar=oaptr->lstarest.prm;
       minptdist=MAXFLOAT;
       while(ar){
        ar2=obptr->lstarest.prm;
        while(ar2){
         Vetor_sub(ar2->p1->pt,ar->p1->pt,testevt);
         if(testevt.norma(&testevt)<minptdist){
          minptdist=testevt.normad;
          arfora1=(Aresta*)ar->aux.prm->end;
          arfora2=ar->ant?(Aresta*)ar->ant->aux.prm->end:(Aresta*)oaptr->lstarest.ult->aux.prm->end;
          ardentro2=(Aresta*)ar2->aux.prm->end;
          ardentro1=ar2->ant?(Aresta*)ar2->ant->aux.prm->end:(Aresta*)obptr->lstarest.ult->aux.prm->end;
         }
         ar2=ar2->prx;
        }
        ar=ar->prx;
       }
       if(arfora1&&arfora2&&ardentro1&&ardentro2)
       {
        dest->lstvert.novo(arfora2->p2);
        dest->lstvert.novo(ardentro2->p1);
        Vetor i,r,s,rd,sd,pv1,pv2;
        Vetor_sub(dest->lstvert.ult->pt,dest->lstvert.ult->ant->pt,i);
        i.normaliza(&i);

        Vetor_sub(ardentro2->p2->pt,ardentro2->p1->pt,r);Vetor::normaliza(&r);
        Vetor_sub(arfora2->p1->pt,arfora2->p2->pt,s);Vetor::normaliza(&s);
        Vetor_sub(ardentro1->p1->pt,ardentro1->p2->pt,rd);Vetor::normaliza(&rd);
        Vetor_sub(arfora1->p2->pt,arfora1->p1->pt,sd);Vetor::normaliza(&sd);

        dest->lstarest.inseredepois(arfora2,dest->lstvert.ult->ant,dest->lstvert.ult,0x21);
        arfora2->p2=dest->lstvert.ult->ant;
        ardentro2->p1=dest->lstvert.ult;
        Aresta *ari,*arptr,*arprx,*arini=0;
        for(ari=obptr->lstarest.prm;ari&&arini==0;ari=ari->prx){
         if(__pontosiguais(ari->p1->pt,ardentro2->p1->pt))
          arini=ari;
        }

        pv1=r^i;
        pv2=rd^i;
        if(Vetor::norma(&pv1)<Vetor::norma(&pv2))
         ardentro1->p2->pt=ardentro1->p2->pt+14*EPS*rd;
        else
         ardentro2->p1->pt=ardentro2->p1->pt+14*EPS*r;

        pv1=s^i;
        pv2=sd^i;
        if(Vetor::norma(&pv1)<Vetor::norma(&pv2))
         arfora1->p1->pt=arfora1->p1->pt+14*EPS*sd;
        else
         arfora2->p2->pt=arfora2->p2->pt+14*EPS*s;


        int ct;
        arprx=arfora2->prx?arfora2->prx:dest->lstarest.ult;
        for(ari=arini,ct=0;ari&&ct<obptr->lstarest.quant;
         ari=ari->prx?ari->prx:obptr->lstarest.prm,ct++){
          if(!ari->aux.prm)continue;
          arptr=(Aresta*)ari->aux.prm->end;

          if(reverteu)
           dest->lstarest.inseredepois(arprx,arptr->p2,arptr->p1);
          else
           dest->lstarest.inseredepois(arprx,arptr->p1,arptr->p2);
          arprx=arprx->prx;
          dest->lstarest.apaga(arptr);
        }
        dest->lstarest.inseredepois(arprx,arprx->p2,arfora1->p1,0x21);
        //  /*
        Aresta *arprx2;
        arprx=arfora1->ant?arfora1->ant:dest->lstarest.ult;
        arprx2=arprx->ant?arprx->ant:dest->lstarest.ult;
        if(FuncoesGeometricas::arestascolineares(arprx,arprx2)){
         Vertice *vu=arprx->p1;
         arprx->p1=arprx2->p1;
         dest->lstarest.apaga(arprx2);
         dest->lstvert.apaga(vu);
        }
        arprx=arfora2->prx?arfora2->prx:dest->lstarest.prm;
        arprx2=arprx->prx?arprx->prx:dest->lstarest.prm;
        if(FuncoesGeometricas::arestascolineares(arprx,arprx2)){
         Vertice *vu=arprx->p2;
         arprx->p2=arprx2->p2;
         dest->lstarest.apaga(arprx2);
         dest->lstvert.apaga(vu);
        }
       }
       // */
      }
     }
    }
   }


  }
  if(dest&&dest->lstarest.ult)
   if(FuncoesGeometricas::arestascolineares(dest->lstarest.ult,dest->lstarest.prm)){
    //Vertice *vu=dest->lstarest.prm->p1;
    dest->lstarest.prm->p1=dest->lstarest.ult->p1;
    Aresta* ult=dest->lstarest.ult->ant;
    dest->lstarest.apaga(dest->lstarest.ult);
    //dest->lstvert.apaga(vu);
    dest->lstarest.ult=ult;
   }
   __UNSETBIT(b->lstarest.flag,AREST_INVERTIDA);

}

template <class TO>
void copia(TO *org,ObjetoLeve *dest){
 dest->lstvert.copia(&org->lstvert);
 dest->lstarest.copia(&dest->lstvert,&org->lstarest);
 dest->borda=org->borda;
}

class JanelaFilhaBase;
template <class TO> TO *Cpgfunc<TO>::booloppoligono(TO *a,TO *b,BOOLOP op,Face *f,JanelaFilhaBase *jfb){
 if(a->lstarest.quant<3||b->lstarest.quant<1)
  return 0;
 TO *dest;
 Aresta *ara,*arb,*arprx;
 Vetor i,j,k,ib,jb,kb;
 bool excluiua,excluiub;
 NDOUBLE alpha, beta;
 Vetor p;
 ObjetoLeve asobrebmais,asobrebmenos,aforab,adentrob,bforaa,bdentroa;
 ObjetoLeve A,B;
 ListaObjetos::alocaeiniciaobjeto(&dest,OBJCPG,false,jfb);
 dest->flag|=OBJTEMPORARIO;
 copia(a,&A);
 copia(b,&B);
 if(f){
  i=f->von_i;k=f->normal;j=f->von_j;
 }
 else FuncoesGeometricas::calcbase(&a->lstarest,&i,&j,&k);
 if(ListaArestas::clockwise(&A.lstarest,&i,&j,&k))
  ListaArestas::revert(&A.lstarest);
 if(ListaArestas::clockwise(&B.lstarest,&i,&j,&k)){
  ListaArestas::revert(&B.lstarest);
  b->lstarest.flag|=AREST_INVERTIDA;
 }
 if(Border3D::sobrepoe(&A.borda.MR,&B.borda.MR)){
  ara=A.lstarest.prm;
  while(ara){
   excluiua=false;
   if(b->borda.Intercepta(ara)){
    arb=B.lstarest.prm;
    while(arb){
     excluiub=false;
     if(a->borda.Intercepta(arb)){
      if(FuncoesGeometricas::arestascolineares(ara,arb)){
       if(Cpg::procarestcolineares(&A,&B,ara,arb,&asobrebmais,&asobrebmenos)){
        arprx=ara->prx;A.lstarest.apaga(ara);ara=arprx;
        B.lstarest.apaga(arb);arb=B.lstarest.prm;
        excluiua=true;
        excluiub=true;
        break;
       }
      }
      else{
       if(FuncoesGeometricas::inters_segmretas3D(&ara->p1->pt,&arb->p1->pt,&ara->p2->pt,&arb->p2->pt,&p,&alpha,&beta)){
        int pos1,pos2;
        pos1=__comp(alpha,0.0,EPS);
        pos2=__comp(alpha,1.0,EPS);
        if(pos1==1&&pos2==-1)
         Cpg::inserealpha(ara,alpha,&p);
        pos1=__comp(beta,0.0,EPS);
        pos2=__comp(beta,1.0,EPS);
        if(pos1==1&&pos2==-1)
         Cpg::inserealpha(arb,beta,&p);
       }
      }
     }
     else{
      Cpgfunc<ObjetoLeve>::adarestaparalista(&bforaa,arb);
      arprx=arb->prx;B.lstarest.apaga(arb);arb=arprx;
      excluiub=true;
     }
     if(!excluiub)
      arb=arb->prx;
    }
    if(!excluiua){
     processaalphas(ara,b,&i,&j,&adentrob,&aforab);
     arprx=ara->prx;A.lstarest.apaga(ara);ara=arprx;
     excluiua=true;
    }
   }
   else{
    Cpgfunc<ObjetoLeve>::adarestaparalista(&aforab,ara);
    arprx=ara->prx;A.lstarest.apaga(ara);ara=arprx;
    excluiua=true;
   }
   if(!excluiua)
    ara=ara->prx;
  }
  for(arb=B.lstarest.prm;arb;arb=arb->prx)
   processaalphas(arb,a,&i,&j,&bdentroa,&bforaa);
 }
 else{
  ara=A.lstarest.prm;
  while(ara){
   Cpgfunc<ObjetoLeve>::adarestaparalista(&aforab,ara);
   ara=ara->prx;
  }
  arb=B.lstarest.prm;
  while(arb){
   Cpgfunc<ObjetoLeve>::adarestaparalista(&bforaa,arb);
   arb=arb->prx;
  }
 }
 ObjetoLeve tempdest;
 geraresultado(op,b,&asobrebmais,&asobrebmenos,&aforab,&adentrob,&bforaa,&bdentroa,&tempdest,&i,&j,&k);
 dest->copia(&tempdest);
 dest->borda.CalculaBordas(&dest->lstvert,0);
 if(a->flag&OBJTEMPORARIO)
  delete a;
 if(b->flag&OBJTEMPORARIO)
  delete b;
 if(dest->lstarest.quant<3){
  delete dest;
  dest=0;
 }
 return dest;
}

#define NUMPARAMS_OBJCPG NUMPARAMS2D+1
#define OBJCPG_arvore    propriedades[NUMPARAMS2D]

#endif
