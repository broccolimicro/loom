.model chip
.inputs a.a call.r
.outputs a.r call.a
.internal
.graph
call.r+ call.a+ 
call.a+ call.a+ 
call.a+ a.r+ 
a.r+ a.a+ 
a.a+ a.r- 
a.r- a.a- 
a.a- call.r- 
call.r- call.a- 
call.a- [call.r];call.a+;(call.a+;a!);[~call.r];call.a-
.marking {<>}
.end
