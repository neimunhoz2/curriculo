#ifndef objetoH
#define objetoH
#include "defs.h"
#include "fonte.h"
#include "linestilo.h"
#include "hatch.h"
#include "materiais.h"
#include "cadio.h"
#include "border.h"

class Transformacao;
class NZbuf;
class Camada;
class Objeto;
class Face;

//poligono
#define INSCRITO     0
#define CIRCUNSCRITO 1
#define LADO         2

#define OBJSELECIONADO 0x01
#define OBJVISIVEL     0x02
#define OBJTEMFONTE    0x04
#define OBJTEMTEXTURA  0x08
#define OBJTEMPORARIO  0x10
#define OBJINVISIVELCSG  0x20
#define OBJRENDERANTIALIASING  0x40
//#define OBJCORBYLAYER          0x80
#define OBJLINEWEIGHTBYLAYER   0x100
#define OBJLINETYPEBYLAYER   0x200
#define OBJHASBULGE   0x400
#define OBJNOBOUNDARY   0x800
#define OBJCORBYBLOCK          0x1000
#define OBJLINETYPEBYBLOCK   0x2000
#define OBJCANPUTINMATLIST   0x4000


class no{
public:
 no(void *col,int ptr);
 no *ant,*prx,*esq,*dir;
 void *colna;
 long endptr;
};

class linhaptr{
public:
 linhaptr();
 linhaptr *ant,*prx;
 no *prmno,*ultno,*atlno;
 Vetor v;//useful in some cases
};

class colunaptr{
public:
 colunaptr();
 colunaptr *ant,*prx;
 no *prmno,*ultno,*atlno;
};

class MatrizPtr{
public:
 MatrizPtr();
 ~MatrizPtr();
 short numcol,numlin;
 MatrizPtr *ant,*prx;
 linhaptr *prmlin,*ultlin,*atllin;
 colunaptr *prmcln,*ultcln,*atlcln;
 no *nos;
 void novacoluna();
 void limpa();
 void apagalinhaptr(linhaptr *lin);
 void novalinhaptr(Vetor *v=0);
 void salva(FILE *arq);
 void abre(FILE *arq);
};

struct VerticeTexP{
 NDOUBLE u,v;
 NDOUBLE r;
};

class InteiroLight{
public:
 InteiroLight *ant,*prx;
 long val;
};

class ListaInteirosLight:public Lista<InteiroLight>{
public:
 void novo(long val){
  novoptr();
  ult->val=val;
 }

 void apagabyval(long val){
  InteiroLight *lp;
  for(lp=prm;lp;lp=lp->prx){
   if(lp->val==val){
    apaga(lp);
    return;
   }
  }
 }

 void copia(ListaInteirosLight *org,long indini=0){
  InteiroLight *lp;
  limpa();
  for(lp=org->prm;lp;lp=lp->prx)
   novo(indini+lp->val);
 }
};

class Inteiro{
public:
 Inteiro *ant,*prx;
 long val,val2;
 char flag;
};

class ListaInteiros:public Lista<Inteiro>{
public:
 void novo(long val,long val2){
  novoptr();
  ult->val=val;
  ult->val2=val2;
  ult->flag=0;
 }
 void inseredepois(Inteiro *anterior,long val,long val2){
  novoptrafter(anterior);
  atual->val=val;
  atual->val2=val2;
 }
 void copia(ListaInteiros *org,long indini=0,long indini2=0){
  Inteiro *lp;
  limpa();
  for(lp=org->prm;lp;lp=lp->prx)
   novo(indini+lp->val,indini2+lp->val2);
 }

 bool apagabyval(long val,long val2){
  Inteiro *lp;
  for(lp=prm;lp;lp=lp->prx){
   if(lp->val==val&&lp->val2==val2){
    apaga(lp);
    return true;
   }
  }
  return false;
 }

 bool exists(long val,long val2=-1){
  Inteiro *lp;
  if(val2!=-1){
   for(lp=prm;lp;lp=lp->prx){
    if(lp->val==val&&lp->val2==val2){
     return true;
    }
   }
  }
  else{
   for(lp=prm;lp;lp=lp->prx){
    if(lp->val==val){
     return true;
    }
   }
  }
  return false;
 }
};



struct Double{
 Double *ant,*prx;
 double val;
};

class ListaReais:public Lista<Double>{
public:
 void novo(double val){
  novoptr();
  ult->val=val;
 }
 void copy(ListaReais &org){
  limpa();
  Double *v;
  for(v=org.prm;v;v=v->prx)
   novo(v->val);
 }
};

struct Long{
 Long *ant,*prx;
 long val;
};

class ListLong:public Lista<Long>{
public:
 ListLong *ant,*prx;
 void novo(long val){
  novoptr();
  ult->val=val;
 }
};

class Vertice{
public:
 Vertice();
 Vertice *ant,*prx;
 Vetor pt,mednorm,ptf;
 Vetor2Df ptelaf;
 Vetor2D ptela;
 VerticeTexP vtex;
 ListaPonteiros faces;
 ListaPonteiros arestas;
 unsigned char FLAG;
};

class ListaVerticesM;

class ListaVertices:public Lista<Vertice>{
public:
 MatrizPtr vertices;
 ~ListaVertices();
 void novo(NDOUBLE x,NDOUBLE y,NDOUBLE z,unsigned char FLAG=VPONTO);
 void novo(Vetor *v,unsigned char FLAG=VPONTO);
 void novo(Vertice *v,unsigned char FLAG=VPONTO);
 void inseredepois(Vertice *anterior,Vetor *p1,unsigned char FLAG=VPONTO);
 void Limpa();
 void copia(ListaVertices *lstOrigem);
 void copia(ListaVerticesM *lstOrigem);
 Vertice *jaexisteverts(Vetor *p);
 static Vertice *procuravertice(void *obj,Vetor *p);
 static Vertice *procuravertice(ListaVertices *lstorg,Vetor *p);
};

class VtxFirstEdge{
public:
 long ar;

 VtxFirstEdge(){
  ar=-1;
 }

 void set(long _ar){
  ar=_ar;
 }

 void set(VtxFirstEdge &vtxffc){
  ar=vtxffc.ar;
 }
};

class VerticeM{
public:
 VerticeM();
 Vetor pt,mednorm,mednormmf,ptf,pttransf;
 Vetor2Df ptelaf;
 Vetor2D ptela;
 VerticeTexP vtex;
 ListaInteirosLight edges;
 VtxFirstEdge firstar;
 unsigned char FLAG;
 static void calcmednormal(Objeto *obj,long indvt);

 VerticeM & operator=(VerticeM &vt){
  pt=vt.pt;
  mednorm=vt.mednorm;
  mednormmf=vt.mednormmf;
  FLAG=vt.FLAG;
  ptf=vt.ptf;
  pttransf=vt.pttransf;
  ptelaf=vt.ptelaf;
  ptela=vt.ptela;
  vtex=vt.vtex;
  firstar.set(vt.firstar);
  edges.limpa();
  edges.copia(&vt.edges);
  return *this;
 }
};


class VerticeEX:public VerticeM{
public:
 VerticeEX():VerticeM(){}
 ColorVet cor;
 bool armazenado;
 Vetor coordlocal,i,j;
 Material* mat;
 VerticeEX & operator=(VerticeEX &vt){
  pt=vt.pt;
  mednorm=vt.mednorm;
  mednormmf=vt.mednormmf;
  FLAG=vt.FLAG;
  ptf=vt.ptf;
  pttransf=vt.pttransf;
  ptelaf=vt.ptelaf;
  ptela=vt.ptela;
  vtex=vt.vtex;
  cor=vt.cor;
  armazenado=vt.armazenado;
  coordlocal=vt.coordlocal;
  i=vt.i;
  j=vt.j;
  mat=vt.mat;
  return *this;
 }
};


class ptrVerticeM{
public:
 VerticeM *vtm;
};

class ListaVerticesM:public ListaM<ptrVerticeM>{
public:
 MatrizPtr vertices;
 ~ListaVerticesM();
 ListaVerticesM();
 bool Aloca(long nitems,bool append);
 void apaga(long ind_item);
 void inivalores(long indvert,Vetor *pt,double tu=0.0,double tv=0.0,unsigned char FLAG=VPONTO);
 void inivalores(long indvert,NDOUBLE x,NDOUBLE y,NDOUBLE z,double tu=0.0,double tv=0.0,unsigned char FLAG=VPONTO);
 void inivalores(long indvert,VerticeM *v,bool copia_arestas);
 void novo(NDOUBLE x=0.0,NDOUBLE y=0.0,NDOUBLE z=0.0,double tu=0.0,double tv=0.0,unsigned char FLAG=VPONTO);
 void novo(Vetor *v,double tu=0.0,double tv=0.0,unsigned char FLAG=VPONTO);
 void novo(VerticeM *v,double tu=0.0,double tv=0.0,unsigned char FLAG=VPONTO);
 void Limpa();
 void copia(ListaVerticesM *lstOrigem,bool copia_arestas=true,bool append=false);//,long indiniar=0, long indinifc=0);
 void copia(ListaVertices *lstOrigem);
 static bool get_adjacent_edges(Objeto* obj,long indvt,ListaInteiros *lstedges);
 static bool get_adjacent_faces(Objeto* obj,long indvt,ListaInteiros *lstfaces);
 static bool get_adjacent_facesEX(Objeto* obj,long indvt,ListaInteiros *lstfaces);
 static long jaexisteverts(Objeto *obj,Vetor *p);
 static VerticeM *procuravertice(Objeto *obj,Vetor *p,long *vtret=0);
 static VerticeM *procuravertice(ListaVerticesM *lstorg,Vetor *p,long *vtret=0);
};



#define AREST_VISIVEL 0x01
#define AREST_TEM_ARREDONDAMENTO 0x02
#define AREST_INVERTIDA 0x04
#define AREST_SELECIONADA 0x08
#define AREST_NULA 0x10
#define POSITIVA (AREST_VISIVEL)
#define NEGATIVA (AREST_VISIVEL|AREST_INVERTIDA)
class Aresta{
public:
 Aresta *ant,*prx;
 Vertice *p1,*p2;
 ListaPonteiros faces,aux;
 Vetor centro;
 Vetor2D p1tela,p2tela;
 unsigned char FLAG;
};
class ListaArestasM;

class ListaArestas:public Lista<Aresta>{
public:
 MatrizPtr arestasV,arestasH;
 char flag;
 ListaArestas();
 ~ListaArestas();
 void novo(Vertice *p1,Vertice *p2,unsigned char FLAG=AREST_VISIVEL);
 void novo(ListaVertices *verts,Vertice *p1,Vertice *p2,unsigned char FLAG=AREST_VISIVEL);
 void inseredepois(Aresta *anterior,Vertice *p1,Vertice *p2,unsigned char FLAG=AREST_VISIVEL);
 void copia(ListaVertices *lstvert,ListaArestas *lstOrigem);
 void copia(ListaVertices *lstvert,ListaArestasM *lstOrigem);
 void Limpa();
 static void revert(ListaArestas *lstar);
 static FVOID centroide(ListaArestas *lstar,Vetor *dest);
 static bool clockwise(ListaArestas *lstar,Vetor *i,Vetor *j,Vetor *k);
 static char jaexistearesta(void *obj,Vertice *v1,Vertice *v2,Aresta **arres);
};

class EdgeVizInfo{
public:
 long fe,fd,arfce,arfcd;
 EdgeVizInfo(){
  fe=fd=arfce=arfcd=-1;
 }
 void Set(long _fe,long _fd,long _arfce,long _arfcd){
  fe=_fe;
  fd=_fd;
  arfce=_arfce;
  arfcd=_arfcd;
 }

 void Set(EdgeVizInfo &org){
  fe=org.fe;
  fd=org.fd;
  arfce=org.arfce;
  arfcd=org.arfcd;
 }
};

class ArestaM{
public:
 long p1,p2;
 ListaPonteiros aux;
 Vetor centro;
 EdgeVizInfo vizinfo;
 Vetor2D p1tela,p2tela;
 unsigned char FLAG;
 ArestaM & operator=(ArestaM &ar){
  p1=ar.p1;
  p2=ar.p2;
  centro=ar.centro;
  FLAG=ar.FLAG;
  p1tela=ar.p1tela;
  p2tela=ar.p2tela;
  vizinfo.Set(ar.vizinfo);
  aux.limpa();
  aux.copia(&ar.aux);
  return *this;
 }
};

class ptrArestaM{
public:
 long index;
 ArestaM *arm;
};

class ListaArestasM:public ListaM<ptrArestaM>{
public:
 MatrizPtr arestasV,arestasH;
 char flag;
 bool Aloca(void *dn,long nitems);
 long apaga(long ind_item);
 void apaga(ArestaM *ar);
 ListaArestasM();
 ~ListaArestasM();
 void zeralistas();
 void inivalores(long indaresta,long p1,long p2,unsigned char FLAG=AREST_VISIVEL);
 void novo(ListaVerticesM *owner,long p1,long p2,unsigned char FLAG=AREST_VISIVEL);
 void novo(ListaVerticesM *owner,VerticeM *p1,VerticeM *p2,unsigned char FLAG=AREST_VISIVEL);
 void copia(ListaVerticesM *owner,ListaArestasM *lstOrigem,long iniindex,long fciniindex=0,bool copyfacesptr=true);
 void copia(ListaVerticesM *owner,ListaArestas *lstOrigem,ListaVertices *lstvertOrigem);
 void Limpa();
 static void revert(ListaArestasM *lstar);
 static FVOID centroide(ListaArestasM *lstar,ListaVerticesM *lstvert,Vetor *dest);
 static char jaexistearesta(Objeto *obj,long v1,long v2,ptrArestaM **arres,long *indarest=0);
 static char jaexistearesta(Objeto *obj,long v1,long v2,long *indarest);
};

#define getptsfromarfc(__p1,__p2,__arfc,__ar) {if((__arfc)->FLAG&AREST_INVERTIDA){__p1=(__ar)->p2;__p2=(__ar)->p1;}\
                                          else{__p1=(__ar)->p1;__p2=(__ar)->p2;}}
#define getp1fromarfc(__p1,__arfc,__ar) {if((__arfc)->FLAG&AREST_INVERTIDA){__p1=(__ar)->p2;}\
                                          else{__p1=(__ar)->p1;}}

#define getleftface(__fc,__arfc,__indv) if(ar->p1==__indv){ __fc=ar->vizinfo.fe; __arfc=ar->vizinfo.arfce;}else{__fc=ar->vizinfo.fd;__arfc=ar->vizinfo.arfcd;}

#define getrightface(__fc,__arfc,__indv) if(ar->p1==__indv){ __fc=ar->vizinfo.fd; __arfc=ar->vizinfo.arfcd;}else{__fc=ar->vizinfo.fe;__arfc=ar->vizinfo.arfce;}

#define setleftface(__fc,__arfc,__indv) if(ar->p1==__indv){ar->vizinfo.fe=__fc; ar->vizinfo.arfce=__arfc;}else{ar->vizinfo.fd=__fc; ar->vizinfo.arfcd=__arfc;}
#define setrightface(__fc,__arfc,__indv) if(ar->p1==__indv){ar->vizinfo.fd=__fc; ar->vizinfo.arfcd=__arfc;}else{ar->vizinfo.fe=__fc; ar->vizinfo.arfce=__arfc;}
class ArestaFace{
public:
 unsigned char FLAG;
 long a1,face;
 Vetor coordlocal;
 VerticeTexP textura_st; 
 ArestaFace & operator=(ArestaFace &ar){
  a1=ar.a1;
  face=ar.face;
  coordlocal=ar.coordlocal;
  FLAG=ar.FLAG;
  textura_st=ar.textura_st;
  return *this;
 }
};

class ptrArestaFace{
public:
 long index;
 ArestaFace *loop;
 ptrArestaFace & operator=(ptrArestaFace &par){
  *loop=*par.loop;
  index=par.index;
  return *this;
 }
};

class ListaFacesM;
class ListaArestaFacesM:public ListaM<ptrArestaFace>{
public:
 bool Aloca(void *dn,long nitems);
 ~ListaArestaFacesM();
 void apaga(int ind_item);
 void inivalores(long indarfc,long a1,long face,unsigned char FLAG=AREST_VISIVEL,NDOUBLE s=0.0,NDOUBLE t=0.0);
 void nova(ListaFacesM *owner,long a1,long face,unsigned char FLAG=AREST_VISIVEL,NDOUBLE s=0.0,NDOUBLE t=0.0);
 void copia(ListaFacesM *__owner,long facedest,ListaArestaFacesM *lstOrigem,long iniindex);
 static bool clockwise(ListaArestaFacesM *lstar,Vetor *i,Vetor *j,Vetor *k);
 void Limpa();
 void revert();
};

#define FACE_VISIVEL 0x01
#define FACE_TEM_TEXTURA 0x02
#define FACE_TEM_NORMALINVERTIDA 0x04
#define FACE_HAS_COLOR 0x08
#define FACE_SELECTED 0x10
#define FACE_ISCONCAVE 0x20

class Face{
public:
 ListaArestaFacesM arestas;
 Vetor centroide,normal,von_i,von_j;
 Material *mat,*bkmat;
 unsigned char FLAG; 

 Face & operator=(Face &fc){
  centroide=fc.centroide;
  normal=fc.normal;
  von_i=fc.von_i;
  von_j=fc.von_j;
  mat=fc.mat;
  bkmat=fc.bkmat;
  FLAG=fc.FLAG;
  return *this;
 }
};

class ptrFace{
public:
 long index;
 Face *fc;
};

class ListaFacesM:public ListaM<ptrFace>{
public:
 bool Aloca(void *dn,long nitems);
 void inivalores(long indfc,unsigned char FLAG=FACE_VISIVEL);
 void nova(Objeto *owner,unsigned char FLAG=FACE_VISIVEL);
 void copia(Objeto *owner,ListaFacesM *lstOrigem,long ariniindex);
 void apaga(long ind_item);
 static void calcnormalface(Objeto *obj,Face *face);
 static void updateparamsface(Objeto *obj,Face *fc);
 void Limpa();
 ~ListaFacesM(){ Limpa(); }
};

#define IMGJPEG 0x01
#define IMGBMP  0x02

class ObjetoLeve;

class Objeto{
public:
 Objeto *ant,*prx,*own;
 Camada *camada;
 unsigned int camadaid;
 JanelaFilhaBase *owner;
 unsigned int flag;
 unsigned short NoDados;
 atnObjectType id;
 PropObj *propriedades;
 ListaVerticesM lstvert;
 ListaVerticesM *lstextra;
 ListaArestasM lstarest;
 ListaFacesM lstfc;
 RetanguloBorda borda;
 ListBordersViewports borders_vports;
 Vetor emf[3];
 ColorVet cCor;
 Objeto(JanelaFilhaBase *jfb);
 ~Objeto();
 void copia(Objeto *objOrigem,bool store_in_mat_list);
 void copia(ObjetoLeve *objOrigem);
 void appendVAF(Objeto *objOrigem);
 static void add_triangle_face(Objeto *dest,Vetor *p1,Vetor *p2,Vetor *p3);
 static void limpa(Objeto *obj);

 static void IniciaNormaisObjeto(Objeto *obj);
 static void CalculaMedNormaisObjeto(Objeto *obj);
 static void faces_transform(Objeto *obj,lst_Transform *lsttransf,bool inversa);
};
typedef char fTestaptDentroPolig(VerticeEX *,int ,VetorLeve *);


class ListaVetoresFast:public ListaFast<Vetor>{
public:
 void novo(double x,double y,double z, unsigned char flag=VPONTO){
  novoptr();
  ult->x=x;ult->y=y;ult->z=z;ult->flag=flag;
 }
};

class ListaReaisFast:public ListaFast<Double>{
public:
 void novo(double val){
  novoptr();
  ult->val=val;
 }
};

struct RayParams{
public:
 double alpha;
 Vetor n,p;
 Objeto *obj;
};

class ListaRayParamsFast:public ListaFast<RayParams>{
public:
 void novo(double alpha,const Vetor &p,const Vetor &n,Objeto *obj){
  novoptr();
  ult->alpha=alpha;
  Vetor_detvalfast2(&ult->p,p.x,p.y,p.z);
  Vetor_detvalfast2(&ult->n,n.x,n.y,n.z);
  ult->obj=obj;
  ult->n.flag=VETOR;
 }
};
class Raio;
class clRayObjInfo{
public:
 Face *fc;
 Vetor *r0,*p0;
 ListaRayParamsFast params;
 Raio *ray;
 NZbuf *zbuf;
 VetorLeve *r;
 void clearlists(){
  params.limpa();
 }
};

typedef FVOIDPTR (*FuncsSelObj)(Objeto *obj,void *arestasret,unsigned int selflag,ListaInteirosLight *validobjs,lst_Transform *lsttransf);
typedef char (*FuncInterceptRayObj)(Objeto *obj,clRayObjInfo *info,bool projconic);
typedef char (*FuncInterceptRayObjV)(Objeto *obj,VerticeEX *vEX,int nvert,clRayObjInfo *info,fTestaptDentroPolig *functesta,bool projconic);

typedef FVOID (*FuncBrdObj)(Objeto *obj,lst_Transform *lsttransf);
typedef FVOID (*FuncTransfObj)(Objeto *obj,lst_Transform *lsttransf,bool inversa);
typedef FVOID (*FuncDesObj)(Objeto *obj,NZbuf* zbuf,lst_Transform *lsttransf);
typedef FINT (*FuncSelPtObj)(Objeto *obj,int X,int Y,int soselprm);

typedef bool (*FuncPrecision)(JanelaFilhaBase* jfb,Objeto *obj,int &X,int &Y,Objeto *ObjIgn,short Ignora,VerticeM* ignorevert);

typedef FVOID (*_FuncInitProps)(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
typedef FVOID (*_FuncChangePts)(Objeto *obj,Vetor *valvo);
typedef FVOID (*FuncAparaObj)(Objeto *,Vetor*);
typedef FVOID (*FuncExplodeObj)(Objeto *);
class VRML97file;
typedef FVOID(*FuncUpdateObj)(Objeto *obj,VRML97file *vrmlfile);
typedef FVOID (*FuncCriaObj)(JanelaFilhaBase *jfb,Objeto *obj,int tipo,VRML97file *vrmlfile);
typedef FVOID (*fMoveObj)(Objeto *obj,Vetor *,Vetor *);

class clFuncInterceptRayObj{
public:
 static char padrao(Objeto *obj,clRayObjInfo *info,bool projconic);
 static char sphere(Objeto *obj,clRayObjInfo *info,bool projconic);
 static char ellipsoid(Objeto *obj,clRayObjInfo *info,bool projconic);
 static char hyperboloid(Objeto *obj,clRayObjInfo *info,bool projconic);
 static char cone(Objeto *obj,clRayObjInfo *info,bool projconic);
 static char cilindro(Objeto *obj,clRayObjInfo *info,bool projconic);
 static char torus(Objeto *obj,clRayObjInfo *info,bool projconic);
 static char csg(Objeto *obj,clRayObjInfo *info,bool projconic);
 static char cuboide(Objeto *obj,clRayObjInfo *info,bool projconic);
 static char needjitter(Objeto *obj,clRayObjInfo *info,double del);
 static char COORDSYSAUX(Objeto *obj,clRayObjInfo *info,bool projconic);
};

class clFuncInterceptRayObjV{
public:
 static char padrao(Objeto *obj,VerticeEX *vEX,int nvert,clRayObjInfo *info,fTestaptDentroPolig *functesta,bool projconic);
 static char sphere(Objeto *obj,VerticeEX *vEX,int nvert,clRayObjInfo *info,fTestaptDentroPolig *functesta,bool projconic);
 static char torus(Objeto *obj,VerticeEX *vEX,int nvert,clRayObjInfo *info,fTestaptDentroPolig *functesta,bool projconic);
 static char ellipsoid(Objeto *obj,VerticeEX *vEX,int nvert,clRayObjInfo *info,fTestaptDentroPolig *functesta,bool projconic);
 static char hyperboloid(Objeto *obj,VerticeEX *vEX,int nvert,clRayObjInfo *info,fTestaptDentroPolig *functesta,bool projconic);
 static char cone(Objeto *obj,VerticeEX *vEX,int nvert,clRayObjInfo *info,fTestaptDentroPolig *functesta,bool projconic);
 static char cilindro(Objeto *obj,VerticeEX *vEX,int nvert,clRayObjInfo *info,fTestaptDentroPolig *functesta,bool projconic);
 static char csg(Objeto *obj,VerticeEX *vEX,int nvert,clRayObjInfo *info,fTestaptDentroPolig *functesta,bool projconic);
 static char cuboide(Objeto *obj,VerticeEX *vEX,int nvert,clRayObjInfo *info,fTestaptDentroPolig *functesta,bool projconic);
 static char COORDSYSAUX(Objeto *obj,VerticeEX *vEX,int nvert,clRayObjInfo *info,fTestaptDentroPolig *functesta,bool projconic);
};
class FuncInitProps{
public:
 static FVOID arc(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID circle(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID cline(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID chart(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID cylinder(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID cone(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID cota(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID drawingboard(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID viewport(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID ellipse(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID face3D(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID fracline(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID hatch(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID image(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID insert(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID line(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID malha3D(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID lightmesh(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID mtext(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID objCPG(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID objCSG(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID paralelepipedo(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID wedge(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID parabola(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID pline(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID point(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID polygon(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID rectangle(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID sphere(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID ellipsoid(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID hyperboloid(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID solid2D(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID spline(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID surface_spline(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID terrain(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID text(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID text3D(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID attdef(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID attrib(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID torus(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID surface_subdivision(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID surf_extrude(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static FVOID COORDSYSAUX(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
};

class DefinicaoObjeto{
public:
 DefinicaoObjeto();
 static void init(DefinicaoObjeto *dest);
 char nome[128];
 FuncInterceptRayObj funcinterceptrayobj;
 FuncInterceptRayObjV funcinterceptrayobjV;
 FuncsSelObj funcsel;
 FuncPrecision funcprecision;
 FuncDesObj funcrend[NUMRENDERTIPOS];
 FuncBrdObj funcborda;
 _FuncInitProps funcinitprops;
 FuncSelPtObj funcselpt;
 FuncUpdateObj funcatualiza;
 _FuncChangePts funcchangepts;
 FuncAparaObj funcapara;
 FuncExplodeObj funcexplode;
 FuncCriaObj funccria;
 funcGravaObjeto funcgrava;
 funcCarregaObjeto funccarrega;
 funcCollisionTest funccollisiontest;
 void *funcreseta;
 FuncTransfObj functransf;
 int NumDados,instancias;
};



#define PROPVISIVEL 0x01
#define PROPOCULTA  0x02
#define PROPNEEDRECRIACAO  0x04
#define PROPNEEDATUALIZACAO  0x08
#define PROPREADONLY  0x10

class LinhaProp{
public:
 char flag;
 LinhaProp();
 ~LinhaProp();
 void aloca(const char *cap,TipoPropObj tp,char flg=PROPVISIVEL,int tprecriacao=0,
  double _minval=0.0,double _maxval=100.0);
 char *caption;
 TipoPropObj tipo;
 int tipo_recriacao;
 double minval,maxval;
};

#define NUM_LANG_SUPPORT 4

struct LangInfo{
 char file[16];
 int righttoleft;
};

class DefinicaoObjetos{
public:
 FVOID init_lang_desc();
 FVOID init(bool reinit=false);
 DefinicaoObjetos();
 ~DefinicaoObjetos();
 void iniciadesc2D(int indobjeto);
 void iniciadesc3D(int indobjeto);
 void iniciadescpropriedades();
 void invoca_atualizacao(Objeto *obj);
 void invoca_transf(Objeto *obj,lst_Transform *lsttransf,bool inversa);
 void invoca_rend(Objeto *obj,NZbuf *zbuf,lst_Transform *lsttransf,int indvis,bool recalccoords);
 DefinicaoObjeto DefObjeto[NumTiposDeObjetos];
 static LangInfo mn_lang_files[NUM_LANG_SUPPORT];
 static LangInfo lang_files[NUM_LANG_SUPPORT];
 static char lang_descr[NUM_LANG_SUPPORT][64];
 LinhaProp **descpropriedades;
 FVOID init_arc();
 FVOID init_circle();
 FVOID init_cline();
 FVOID init_chart();
 FVOID init_cylinder();
 FVOID init_cone();
 FVOID init_cota();
 FVOID init_drawingboard();
 FVOID init_viewport();
 FVOID init_ellipse();
 FVOID init_face3D();
 FVOID init_fraclin();
 FVOID init_hatch();
 FVOID init_image();
 FVOID init_insert();
 FVOID init_line();
 FVOID init_malha3D();
 FVOID init_lightmesh();
 FVOID init_mtext();
 FVOID init_attdef();
 FVOID init_attrib();
 FVOID init_objCPG();
 FVOID init_objCSG();
 FVOID init_paralelepipedo();
 FVOID init_wedge();
 FVOID init_pline();
 FVOID init_point();
 FVOID init_polygon();
 FVOID init_parabola();
 FVOID init_ray();
 FVOID init_rectangle();
 FVOID init_sphere();
 FVOID init_ellipsoid();
 FVOID init_hyperboloid();
 FVOID init_solid2D();
 FVOID init_spline();
 FVOID init_surface_spline();
 FVOID init_terrain();
 FVOID init_text();
 FVOID init_text3D();
 FVOID init_torus();
 FVOID init_surface_subdivision();
 FVOID init_surf_extrude();
 FVOID init_COORDSYSAUX();
};



class ListaObjetos:public Lista<Objeto>{
public:
 void Limpa(short SoObjeto=0);
 Objeto *copia(Objeto *objOrigem,bool store_in_mat_list);
 void clone(ListaObjetos *org);
 void novo(JanelaFilhaBase *jfb,atnObjectType tipo);
 void novo(JanelaFilhaBase *jfb,Objeto *obj);
 static void  CopiaDadosPadrao2D(Objeto *org,Objeto *dest,bool putinmatlist);
 static void  ini_name_and_color(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static void  IniciaDadosPadraoA(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static void  IniciaDadosPadrao3D(Objeto *obj);
 static void  iniciapropriedades(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static void alocapropriedades(Objeto *obj);
 static void changeType(Objeto *obj,atnObjectType newtype,JanelaFilhaBase* jfb);
 static void updateTopologyInfo(Objeto *obj);
 static void  alocaeiniciapropriedades(Objeto *obj,bool putinmatlist,JanelaFilhaBase* jfb);
 static bool  alocaeiniciaobjeto(Objeto **obj,atnObjectType tipo,bool putinmatlist,JanelaFilhaBase* jfb);
 static void  alocaeiniciaobjeto(ObjetoLeve **obj,atnObjectType tipo,bool putinmatlist,JanelaFilhaBase* jfb);
 static void  IniciaDadosObjetos(ListaObjetos *lstobjs,JanelaFilhaBase *jfb,Objeto **obj,bool atualizavista=true,bool addtoundo=true);
 static void ad_linha(ListaObjetos *dest,Vetor *p1,Vetor *p2,bool putinmatlist,JanelaFilhaBase *jfb);
 static void ad_solido2d(ListaObjetos *dest,Vetor *pts,int npts,bool putinmatlist,JanelaFilhaBase *jfb);
 static void ad_arc(ListaObjetos *dest,Vetor *pini,Vetor *pfim,Vetor *normal,Vetor *centro,double raio,bool putinmatlist,JanelaFilhaBase *jfb);
 static void ad_circle(ListaObjetos *dest,int nseg,double angini,double angfim,Vetor *normal,Vetor *centro,double raio,bool putinmatlist,JanelaFilhaBase *jfb);
 static void ad_text(ListaObjetos *dest,Vetor *ptins,const char *text,Vetor *i,Vetor *j,double altura,int txt_align,int transp,bool putinmatlist,JanelaFilhaBase *jfb);
 static void ad_attdef(ListaObjetos *dest,Vetor *ptins,char *label,char *text,int flags,double alttext,
  Vetor *i,Vetor *j,int txt_align,bool putinmatlist,JanelaFilhaBase *jfb);
 static void ad_attrib(ListaObjetos *dest,Vetor *ptins,char *attdef,char *text,int flags,double alttext,Vetor *i,Vetor *j,int txt_align,bool putinmatlist,JanelaFilhaBase *jfb);
};


struct Poligono
{
 Poligono *ant,*prx;
 ListaVerticesM vertices;
 Vetor normal;
};

class ListaPoligonos:public Lista<Poligono>{
public:
 void *usrdado;
 ListaPoligonos *prx;
 void novo(Objeto *obj,Face *fc);
 static bool clockwise(Poligono *pol);
 void append(ListaPoligonos *lstOrigem);
 void novo(Poligono *pol);
 void criamundo(ListaObjetos *objs);
 void criaobjeto(Objeto *obj);
};

class ObjetoLeve{
public:
 ObjetoLeve *ant,*prx;
 char flag;
 void *usrdado;
 void *usrdado2;
 ObjetoLeve(){
  usrdado2=0;usrdado=0;flag=0;ant=prx=0;
 }
 ~ObjetoLeve(){
  limpa();
 }
 ListaVertices lstvert;
 ListaArestas lstarest;
 RetanguloBorda borda;
 template <class TO>
 friend void copia(TO *org,ObjetoLeve *dest);
 void copia(Poligono *pol);
 void copia(Objeto *obj,long fc);
 void copia(ObjetoLeve *pol);
 void limpa();
};

class ListaObjetoLeve:public Lista<ObjetoLeve>
{
public:
 ListaObjetoLeve *prx;
 char flag;
 void ad(Objeto *obj,long fc);
 void ad(ObjetoLeve *ol);
 void criaobjeto(Objeto *obj);
};

#endif
