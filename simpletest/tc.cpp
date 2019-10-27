// 1568693954-60704944

#include <bits/stdc++.h>
#define MOD1 1000000007
#define MOD2 1004535809
#define FR first
#define SE second

using namespace std;

typedef long long ll;
typedef pair<int,int> pr;

const ll H=100456;

ll pow1[1000005],pow2[1000005],powd[1000005];

void pre() {
  vector<int> vi;
  pow1;
  pow1[0]=pow2[0]=powd[0]=1;
  for(int i=1;i<=1e6;i++) {
  	pow1[i]=pow1[i-1]*H%MOD1;
  	pow2[i]=pow2[i-1]*H%MOD2;
  	powd[i]=powd[i-1]*10LL%MOD1;
  }
}

namespace SGT {

const int Maxn=15000000;
int testm=0;

int sumv1[Maxn],sumv2[Maxn];
int ch[Maxn][2],tot;

inline int cpynode(int o) {
  tot++;
  sumv1[tot]=sumv1[o];sumv2[tot]=sumv2[o];
  ch[tot][0]=ch[o][0];ch[tot][1]=ch[o][1];
  return tot;
}

void pushup(int l,int o) {
  sumv1[o]=(sumv1[ch[o][0]]+pow1[l]*sumv1[ch[o][1]])%MOD1;
  sumv2[o]=(sumv2[ch[o][0]]+pow2[l]*sumv2[ch[o][1]])%MOD2;
}

int update(int l,int r,int o,int p,int q) {
  o=cpynode(o);
  if (l==r) {
  	sumv1[o]=sumv2[o]=q;
  	return o;
  }
  else {
  	int m=((l+r)>>1);
  	if (m>=p) ch[o][0]=update(l,m,ch[o][0],p,q);
  	else ch[o][1]=update(m+1,r,ch[o][1],p,q);
  	pushup(m-l+1,o);
  	return o;
  }
}

bool query(int l,int r,int x,int y) {
  if (sumv1[x]==sumv1[y]&&sumv2[x]==sumv2[y]) return 0;
  if (l==r) return sumv1[x]<sumv1[y];
  else {
  	int m=((l+r)>>1);
  	if (sumv1[ch[x][0]]==sumv1[ch[y][0]]&&sumv2[ch[x][0]]==sumv2[ch[y][0]])
  	  return query(m+1,r,ch[x][1],ch[y][1]);
  	else return query(l,m,ch[x][0],ch[y][0]);
  }
}

}

struct Data {
  int x,d,rt;
  Data() {}
  Data(int a,int b,int c):x(a),d(b),rt(c) {}
  bool operator < (const Data & b) const {
  	if (d!=b.d) return d>b.d;
  	return x>b.x;
  }
};

Data trans(Data x,int u,int v) {
  int cur[10],cnt=0;
  x.x=u;
  while (v) {
  	cur[++cnt]=v%10;
  	v/=10;
  }
  for(int i=1;i<=cnt;i++) x.rt=SGT::update(1,6e5,x.rt,++x.d,cur[cnt-i+1]);
  return x;
}

bool cmp(Data x,Data y) {
  if (x.d!=y.d) return x.d<y.d;
  return SGT::query(1,6e5,x.rt,y.rt);
}

Data dis[100005];
priority_queue <Data> q;

vector <pr> e[100005];
int len[100005],p[100005],pl[100005];

void dijkstra(int n,int m) {
  for(int i=1;i<=m;i++) len[i]=len[i/10]+1;
  for(int i=2;i<=n;i++) dis[i]=Data(i,1e9,0);
  dis[1]=Data(1,0,0);
  q.push(dis[1]);
  while (!q.empty()) {
  	Data t=q.top();q.pop();
  	if (cmp(dis[t.x],t)) continue;
  	int x=t.x,d=t.d;
  	for(int i=0;i<e[x].size();i++) {
  		int u=e[x][i].FR,v=e[x][i].SE;
  		if (d+len[v]<dis[u].d) {
  			dis[u]=trans(dis[x],u,v);
  			p[u]=x;pl[u]=v;
  			q.push(dis[u]);
		  }
		else if (d+len[v]==dis[u].d) {
			Data t=trans(dis[x],u,v);
			if (cmp(t,dis[u])) {
				dis[u]=t;
				p[u]=x;pl[u]=v;
				q.push(dis[u]);
			}
		}
	  }
  }
}

int ans[100005];
bool vis[100005];

int solve(int x) {
  if (vis[x]) return ans[x];
  vis[x]=1;
  return ans[x]=(solve(p[x])*powd[len[pl[x]]]+pl[x])%MOD1;
}

int main() {
  int n,m;
  scanf("%d%d",&n,&m);
  pre();
  for(int i=1;i<=m;i++) {
  	int x,y;
  	scanf("%d%d",&x,&y);
  	e[x].push_back(pr(y,i));
  	e[y].push_back(pr(x,i));
  }
  dijkstra(n,m);
  vis[1]=1;
  for(int i=2;i<=n;i++) printf("%d\n",solve(i));
  return 0;
}
