.model mux1b2
.inputs A.r.f A.r.t B.r.f B.r.t O.a S.r.f S.r.t call.r
.outputs A.a B.a O.r.f O.r.t S.a call.a
.internal o s
.graph
call.r+ call.a+ 
call.a+ 

1->
S.r.f- S.r.t+ 
S.r.f- S.r.t+ s+ 
s+ s+
s- 
s- s-
s+S.a+ 
s-S.a+ 
S.a+ S.r.f- S.r.t- 
S.r.f- S.r.t- S.a- 
S.a- s- s+ 
s- 
A.r.f- A.r.t+ 
A.r.f- A.r.t+ o+ 
o+ o+
o- 
o- o-
o+A.a+ 
o-A.a+ 
A.a+ A.r.f- A.r.t- 
A.r.f- A.r.t- A.a- 
A.a- o+ o- 
o+ O.r.t+ O.r.f- 
O.r.t+ O.r.f- O.r.t,O.r.f:=1,0
o- O.r.t- O.r.f+ 
O.r.t- O.r.f+ O.r.t,O.r.f:=0,1
O.r.t,O.r.f:=1,0O.a+ 
O.r.t,O.r.f:=0,1O.a+ 
O.a+ O.r.t- O.r.f- 
O.r.t- O.r.f- O.a- 
O.a- A?o;O!o
s+ 
B.r.f- B.r.t+ 
B.r.f- B.r.t+ o+ 
o+ o+
o- 
o- o-
o+B.a+ 
o-B.a+ 
B.a+ B.r.f- B.r.t- 
B.r.f- B.r.t- B.a- 
B.a- o+ o- 
o+ O.r.t+ O.r.f- 
O.r.t+ O.r.f- O.r.t,O.r.f:=1,0
o- O.r.t- O.r.f+ 
O.r.t- O.r.f+ O.r.t,O.r.f:=0,1
O.r.t,O.r.f:=1,0O.a+ 
O.r.t,O.r.f:=0,1O.a+ 
O.a+ O.r.t- O.r.f- 
O.r.t- O.r.f- O.a- 
O.a- B?o;O!o
A?o;O!o1->Block
B?o;O!o1->Block
1->Block1->call.r- 
call.r- call.a- 
call.a- [call.r];call.a+;(int<1>s;int<1>o;*[S?s;[~s->A?o;O!o[]s->B?o;O!o];]);[~call.r];call.a-
.marking {<>}
.end
