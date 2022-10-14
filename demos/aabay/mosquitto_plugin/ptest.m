JSONTEST0    ;
	; Input: Single string (callback in ^JSONPARSER)
	; Output: All scalar values and all (nested) property names
	;
	k a s a=$P($T(txt),";",2,2000)
	s a("callback","skalar")="cbskalar^JSONTEST0",a("callback","start")="cbstart^JSONTEST0",a("callback","end")="cbend^JSONTEST0"
	s a("callback","getc")="getc",a("callback","ungetc")="ungetc"
	s result=$$^JSONPARSER(.a) w "result:",result,!
	q

txt ;{"a1": 123.1, "a2": [1,2], "a3": {"a":"1"}}
	;{"Hobbies":["music","cinema"],"NN":"Mustermann","VN":"Hans","adresse":{"Ort":"Stuttgart","PLZ":70374} }
	;{"by":1967,"nn":"Mustermann","children": ["child-1","child-2"],"male":true,"female":false,"div":null }

	; These callback-functions are also used by JSONTEST3 and JSONTEST4 !!!
cbskalar(l,t,txt)
	n (l,t,txt)
	w t," ",txt,!
	q

cbstart(l,txt)
	n (l,txt)
	w "start "_$S($D(txt):"prop "_txt,1:"array"),!
	q

cbend(l,txt)
	w "end "_$S($D(txt):"prop "_txt,1:"array"),!
	q
