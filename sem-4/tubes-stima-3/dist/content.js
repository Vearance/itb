var e=Object.defineProperty,t=Object.getOwnPropertyDescriptor,n=Object.getOwnPropertyNames,r=Object.prototype.hasOwnProperty,i=(e,t)=>()=>(e&&(t=e(e=0)),t),a=(e,t)=>()=>(t||(e((t={exports:{}}).exports,t),e=null),t.exports),o=(t,n)=>{let r={};for(var i in t)e(r,i,{get:t[i],enumerable:!0});return n||e(r,Symbol.toStringTag,{value:`Module`}),r},s=(i,a,o,s)=>{if(a&&typeof a==`object`||typeof a==`function`)for(var c=n(a),l=0,u=c.length,d;l<u;l++)d=c[l],!r.call(i,d)&&d!==o&&e(i,d,{get:(e=>a[e]).bind(null,d),enumerable:!(s=t(a,d))||s.enumerable});return i},c=t=>r.call(t,`module.exports`)?t[`module.exports`]:s(e({},`__esModule`,{value:!0}),t),l=(e=>typeof require<`u`?require:typeof Proxy<`u`?new Proxy(e,{get:(e,t)=>(typeof require<`u`?require:e)[t]}):e)(function(e){if(typeof require<`u`)return require.apply(this,arguments);throw Error('Calling `require` for "'+e+"\" in an environment that doesn't expose the `require` function. See https://rolldown.rs/in-depth/bundling-cjs#require-external-modules for more details.")}),u=`judol-detector-highlight`,d=`judol-detector-tooltip`,f=`judol-tooltip`,p=`judol-tooltip-style`,m=`judol-tooltip__row`,h=`judol-detector-style`,g=`judol-image`,_=`judol-text-blur`,v=`judol-scan-stats`,y=`judol-detector-settings`,b=new Set([`SCRIPT`,`STYLE`,`NOSCRIPT`,`TEXTAREA`,`INPUT`,`SELECT`,`OPTION`,`CODE`,`PRE`,`SVG`,`CANVAS`]),x=/\S/u;function S(e){let t=[],n=document.createTreeWalker(e,NodeFilter.SHOW_TEXT,{acceptNode(e){return e instanceof Text&&!w(e)?NodeFilter.FILTER_ACCEPT:NodeFilter.FILTER_REJECT}}),r=n.nextNode();for(;r!==null;)r instanceof Text&&t.push(r),r=n.nextNode();return t}function C(e){if(e.type===`attributes`)return e.target instanceof HTMLImageElement||!T(e.target);if(e.type===`characterData`)return!T(e.target);let t=[...Array.from(e.addedNodes),...Array.from(e.removedNodes)];return t.length===0?!T(e.target):t.some(e=>!T(e))}function w(e){if(!x.test(e.data))return!0;let t=e.parentElement;return t===null||b.has(t.tagName)||t.id===`judol-detector-tooltip`?!0:t.closest(`#${d}`)!==null}function T(e){let t=e instanceof Element?e:e.parentElement;return t?t.classList.contains(`judol-detector-highlight`)||t.classList.contains(`judol-image`)||t.classList.contains(`judol-text-blur`)||t.classList.contains(`judol-tooltip`)||t.closest(`.judol-detector-highlight, .judol-image, .judol-text-blur, .judol-tooltip`)!==null:!1}var E=/(^|[^a-z0-9])([a-z][a-z\d]*[a-z]\d{2,3})(?![a-z0-9])/gu;function D(e,t){let n=performance.now(),r=[];E.lastIndex=0;try{let n=E.exec(t.value);for(;n!==null;){let i=n[1]??``,a=n[2]??``,o=n.index+i.length,s=o+a.length,c=t.map[o],l=t.map[s-1];c!==void 0&&l!==void 0&&r.push({start:c.start,end:l.end,keyword:e.slice(c.start,l.end),canonicalKeyword:a,algorithm:`RegEx`}),E.lastIndex===n.index&&(E.lastIndex+=1),n=E.exec(t.value)}}finally{E.lastIndex=0}return{matches:r,executionMs:performance.now()-n}}var ee=[`a`,`button`,`label`,`p`,`li`,`h1`,`h2`,`h3`,`h4`,`h5`,`h6`,`td`,`th`,`blockquote`,`figcaption`,`[role='button']`,`[role='heading']`,`[role='link']`].join(`, `),O=new Set([`HTML`,`BODY`]);function k(){if(document.getElementById(`judol-detector-style`))return;let e=document.createElement(`style`);e.id=h,e.textContent=`
        .${u} {
            background: rgba(255, 214, 10, 0.68);
            border-bottom: 2px solid #d9480f;
            border-radius: 3px;
            box-decoration-break: clone;
            -webkit-box-decoration-break: clone;
            color: inherit;
            cursor: help;
            padding: 0 0.05em;
        }

        img.${g}[data-judol-action="blur"] {
            cursor: help;
            filter: blur(10px) !important;
            transition: filter 0.2s ease;
        }

        .${_}[data-judol-action="text-blur"] {
            cursor: help;
            filter: blur(6px) !important;
            transition: filter 0.18s ease;
        }
    `,document.head.appendChild(e)}function A(e,t){e.dataset.keyword=t.keyword,e.dataset.algorithm=t.algorithm,e.dataset.occurrences=String(t.occurrences),e.dataset.executionTime=t.executionTime.toFixed(3)}function j(e=document.body){let t=Array.from(e.querySelectorAll(`.${u}`)),n=new Set;for(let e of t){let t=e.parentNode;if(t){for(;e.firstChild;)t.insertBefore(e.firstChild,e);t.removeChild(e),n.add(t)}}for(let e of n)e.normalize()}function te(e,t=document.body){let n=new Map;for(let r of e){if(!r.node.isConnected)continue;let e=le(r.node);if(!e||!fe(t,e))continue;let i=n.get(e);(!i||re(r,i)<0)&&n.set(e,r)}let r=new Set(se(t));for(let e of r)n.has(e)||ce(e);for(let[e,t]of n)e.classList.add(_),e.dataset.judolAction=`text-blur`,A(e,t)}function M(e=document.body){let t=se(e);for(let e of t)ce(e)}function ne(e){let t=new Map;for(let n of e){let e=t.get(n.node)??[];e.push(n),t.set(n.node,e)}let n=[];for(let e of t.values()){let t=[],r=[...e].sort(re);for(let e of r)t.some(t=>pe(e,t))||t.push(e);n.push(...t.sort((e,t)=>e.startIndex-t.startIndex))}return n}function re(e,t){let n=N(e)-N(t);if(n!==0)return n;let r=P(t)-P(e);return r===0?e.startIndex-t.startIndex:r}function N(e){return e.algorithm===`RegEx`?0:e.algorithm===`Weighted Levenshtein`?2:1}function P(e){return e.endIndex-e.startIndex}function ie(e,t=document.body){let n=new Set(Array.from(t.querySelectorAll(`.${u}`))),r=new Set,i=new Set,a=new Map;for(let t of e){let e=a.get(t.node)??[];e.push(t),a.set(t.node,e)}for(let[e,t]of a){let n=e.nodeValue??``,a=e.parentNode;if(!a||n.length===0||!e.isConnected)continue;let o=t.sort((e,t)=>e.startIndex-t.startIndex),s=a instanceof HTMLElement&&a.classList.contains(`judol-detector-highlight`)?a:null;if(s){if(o.length===1&&o[0].startIndex===0&&o[0].endIndex===n.length){A(s,o[0]),r.add(s);continue}let e=ae(n,o),t=s.parentNode;t&&(t.replaceChild(e,s),i.add(t));continue}let c=ae(n,o,r);a.replaceChild(c,e),i.add(a)}for(let e of n)if(!r.has(e)&&e.isConnected){let t=oe(e);t&&i.add(t)}for(let e of i)e.normalize()}function ae(e,t,n){let r=document.createDocumentFragment(),i=0;for(let a of t){if(a.startIndex<i||a.endIndex>e.length)continue;a.startIndex>i&&r.appendChild(document.createTextNode(e.slice(i,a.startIndex)));let t=document.createElement(`span`);t.className=u,t.textContent=e.slice(a.startIndex,a.endIndex),A(t,a),r.appendChild(t),n?.add(t),i=a.endIndex}return i<e.length&&r.appendChild(document.createTextNode(e.slice(i))),r}function oe(e){let t=e.parentNode;if(!t)return null;for(;e.firstChild;)t.insertBefore(e.firstChild,e);return t.removeChild(e),t}function se(e){let t=Array.from(e.querySelectorAll(`.${_}`));return e instanceof HTMLElement&&e.classList.contains(`judol-text-blur`)&&t.unshift(e),t}function ce(e){e.classList.remove(_),e.dataset.judolAction===`text-blur`&&(delete e.dataset.judolAction,delete e.dataset.keyword,delete e.dataset.algorithm,delete e.dataset.occurrences,delete e.dataset.executionTime)}function le(e){let t=e.parentElement;for(;t?.classList.contains(u);)t=t.parentElement;if(!t||de(t)||F(t))return null;let n=t.closest(ee);if(n&&!de(n)&&!F(n))return n;let r=t;for(;r&&!de(r);){if(!F(r)&&ue(r))return r;r=r.parentElement}return F(t)?null:t}function ue(e){let t=window.getComputedStyle(e).display;return t!==`inline`&&t!==`contents`}function de(e){return O.has(e.tagName)}function F(e){return e.classList.contains(`judol-detector-highlight`)||e.classList.contains(`judol-image`)}function fe(e,t){return e instanceof Node?e.contains(t):!0}function pe(e,t){return e.startIndex<t.endIndex&&t.startIndex<e.endIndex}var I=me(`slot\r
slot gacor\r
gacor\r
maxwin\r
scatter\r
scatter hitam\r
scatter merah\r
jackpot\r
jackpot maxwin\r
bonus new member\r
bonus member baru\r
bonus deposit\r
bonus harian\r
bonus mingguan\r
bonus bulanan\r
bonus rollingan\r
bonus turnover\r
bonus referral\r
bonus cashback\r
cashback slot\r
cashback casino\r
free spin\r
free spins\r
freebet\r
free bet\r
freechip\r
free chip\r
deposit pulsa\r
deposit ewallet\r
deposit dana\r
deposit ovo\r
deposit gopay\r
deposit qris\r
deposit tanpa potongan\r
minimal deposit\r
wd cepat\r
withdraw cepat\r
withdraw kilat\r
withdraw otomatis\r
rtp\r
rtp live\r
rtp slot\r
rtp gacor\r
rtp tertinggi\r
rtp hari ini\r
slot online\r
judi online\r
situs judi\r
situs slot\r
bandar slot\r
bandar togel\r
casino online\r
live casino\r
pragmatic play\r
pg soft\r
habanero\r
joker gaming\r
microgaming\r
spadegaming\r
slot88\r
slot 88\r
slot777\r
slot 777\r
slot gacor hari ini\r
slot gampang menang\r
slot terpercaya\r
slot resmi\r
slot terbaru\r
slot terbaik\r
slot viral\r
slot thailand\r
slot olympus\r
olympus1000\r
olympus 1000\r
mahjong ways\r
mahjong ways 2\r
sweet bonanza\r
starlight princess\r
gates of olympus\r
gates olympus\r
wild west gold\r
bonanza slot\r
zeus slot\r
kakek zeus\r
petir zeus\r
slot petir\r
slot receh\r
slot anti rungkad\r
anti rungkad\r
anti kalah\r
pasti menang\r
jamin menang\r
cuan slot\r
cuan besar\r
cuan maxwin\r
cuan gacor\r
slot cuan\r
situs gacor\r
game gacor\r
akun gacor\r
jam gacor\r
pola gacor\r
pola slot\r
pola maxwin\r
jam hoki\r
jam hoki slot\r
jam hoki gacor\r
admin slot\r
link alternatif\r
link gacor\r
login slot\r
daftar slot\r
daftar judi\r
register slot\r
main slot\r
main judi\r
main casino\r
main togel\r
pasang togel\r
prediksi togel\r
angka hoki\r
angka jitu\r
angka keramat\r
slot deposit pulsa\r
slot via dana\r
slot via ovo\r
slot via qris\r
slot via gopay\r
slot deposit qris\r
slot tanpa potongan\r
slot resmi terpercaya\r
situs terpercaya\r
agen judi\r
agen slot\r
agen togel\r
bandar darat online\r
bandar online\r
slot resmi indonesia\r
slot gampang wd\r
slot gampang maxwin\r
slot no 1\r
slot nomor 1\r
slot paling gacor\r
slot paling cuan\r
slot hoki\r
slot hoki hari ini\r
slot gacor maxwin\r
slot gacor terpercaya\r
slot gacor terbaru\r
slot gacor 2026\r
slot gacor 2025\r
slot thailand gacor\r
slot server thailand\r
server thailand\r
server kamboja\r
slot server kamboja\r
slot luar negeri\r
akun pro slot\r
akun sultan\r
akun hoki\r
akun maxwin\r
situs anti rungkad\r
judi bola\r
taruhan bola\r
sportsbook\r
mix parlay\r
parlay\r
casino terpercaya\r
roulette online\r
baccarat online\r
blackjack online\r
poker online\r
capsa online\r
domino qiu qiu\r
dominoqq\r
qq online\r
bandarq\r
sakong online\r
aduq online\r
togel singapore\r
togel hongkong\r
togel sidney\r
togel macau\r
toto gelap\r
togel online\r
pasaran togel\r
angka togel\r
keluaran togel\r
result togel\r
prediksi hk\r
prediksi sdy\r
prediksi sgp\r
live draw\r
live draw hk\r
live draw sdy\r
live draw sgp\r
jackpot terbesar\r
jackpot harian\r
jackpot mingguan\r
bonus member baru 100\r
bonus 100\r
bonus 200\r
bonus cashback 10 persen\r
bonus turnover slot\r
bonus rollingan slot\r
bonus referral slot\r
slot bonus new member\r
casino bonus\r
casino online terpercaya\r
situs gacor terpercaya\r
agen terpercaya\r
daftar sekarang\r
main sekarang\r
claim bonus\r
klaim bonus\r
spin gratis\r
spin free\r
spin otomatis\r
auto spin\r
buy free spin\r
buy feature\r
feature buy\r
slot buy feature\r
big win\r
mega win\r
super win\r
sensational\r
epic win\r
double chance\r
turbo spin\r
auto cuan\r
modal receh\r
modal kecil cuan besar\r
deposit receh\r
deposit 10 ribu\r
deposit 5 ribu\r
deposit murah\r
situs resmi slot\r
situs slot online terpercaya\r
situs slot gacor terpercaya\r
slot online terpercaya no 1\r
slot online gacor\r
slot online maxwin\r
slot online gampang menang\r
slot online indonesia\r
slot online via dana\r
slot online deposit pulsa\r
slot qris\r
slot dana\r
slot ovo\r
slot gopay\r
slot pulsa\r
slot tri\r
slot xl\r
slot telkomsel\r
slot indosat\r
slot smartfren\r
judi slot online\r
judi casino online\r
judi terpercaya\r
judi gacor\r
judi bola online\r
judi togel online\r
judi deposit pulsa\r
slot deposit dana\r
slot deposit ovo\r
slot deposit gopay\r
slot deposit linkaja\r
slot deposit shopeepay\r
situs slot deposit pulsa tanpa potongan\r
slot deposit ewallet\r
judi online terpercaya\r
akun demo slot\r
demo slot\r
provider slot\r
provider gacor\r
provider terbaik\r
slot pragmatic\r
slot pg soft\r
slot habanero\r
slot joker\r
slot microgaming\r
slot spadegaming\r
slot live22\r
slot toptrend\r
slot yggdrasil\r
slot jdb\r
slot cq9\r
slot playstar\r
slot ion\r
slot onetouch\r
slot no limit city\r
slot hacksaw\r
slot dragon hatch\r
slot aztec\r
slot zeus\r
slot fu fu fu\r
slot panda\r
slot naga\r
slot kakek merah\r
slot princess\r
slot bonanza\r
slot aladdin\r
slot buah\r
slot ikan\r
slot dewa petir\r
slot dewa zeus\r
slot gacor malam ini\r
slot gacor sore ini\r
slot gacor pagi ini\r
slot gacor siang ini\r
slot gacor auto maxwin\r
slot gacor anti kalah\r
slot gacor hari ini terpercaya\r
slot gacor gampang menang\r
slot gacor modal kecil\r
slot gacor deposit kecil\r
slot gacor bonus besar\r
slot gacor terpercaya no 1\r
situs slot gacor no 1\r
situs slot gampang menang\r
situs judi terpercaya no 1\r
link slot gacor\r
link judi online\r
link slot online\r
alternatif slot\r
alternatif judi\r
mirror site slot\r
mirror judi\r
main slot sekarang\r
main judi sekarang\r
main dan menang\r
auto menang\r
auto wd\r
wd tanpa syarat\r
wd 1 menit\r
wd 5 menit\r
wd tercepat\r
slot anti zonk\r
anti zonk\r
slot gacor anti zonk\r
slot hoki malam ini\r
slot hoki hari ini\r
slot hoki gacor\r
admin gacor\r
admin maxwin\r
admin hoki\r
jackpot terbesar hari ini\r
slot jackpot terbesar\r
slot jackpot maxwin\r
casino gacor\r
live casino gacor\r
live slot\r
live jackpot\r
prediksi slot\r
trik slot\r
cara menang slot\r
cara maxwin\r
tips gacor\r
tips maxwin\r
bocoran slot\r
bocoran admin\r
bocoran jam gacor\r
bocoran pola slot\r
bocoran maxwin\r
akun sultan gacor\r
akun hoki maxwin\r
akun gacor terpercaya\r
akun anti rungkad\r
akun auto win\r
akun auto maxwin\r
slot vip\r
member vip\r
vip slot\r
vip casino\r
vip gambling\r
high roller\r
rollingan besar\r
turnover besar\r
bonus turnover casino\r
bonus cashback harian\r
cashback mingguan\r
cashback bulanan\r
judi terpercaya indonesia\r
slot resmi terbaik\r
slot resmi no 1\r
slot resmi gampang menang\r
judi slot terpercaya\r
judi slot gacor\r
judi slot maxwin\r
slot online gampang wd\r
slot online gampang cuan\r
slot online anti kalah\r
slot online anti rungkad\r
slot online terpercaya indonesia\r
situs slot resmi indonesia\r
situs judi online indonesia\r
slot online terbaik\r
slot online resmi\r
slot online bonus besar\r
slot online bonus new member\r
casino online indonesia\r
casino online terpercaya indonesia\r
situs casino online\r
bandar casino online\r
bandar togel online\r
bandar slot online`);function me(e){return e.split(/\r?\n/u).map(e=>e.trim()).filter(e=>e.length>0)}function he(e){let t=e.length,n=Array(t).fill(0),r=0,i=1;for(;i<t;)e[i]===e[r]?(r++,n[i]=r,i++):r===0?(n[i]=0,i++):r=n[r-1];return n}function ge(e,t,n=!1){let r=performance.now();if(t.length===0||e.length===0||t.length>e.length)return{matched:!1,matchIndexes:[],comparisonCount:0,executionTime:performance.now()-r,algorithm:`KMP`};let i=e,a=t;n||(i=e.toLowerCase(),a=t.toLowerCase());let o=i.length,s=a.length,c=he(a),l=[],u=0,d=0,f=0;for(;d<o;)u++,i[d]===a[f]?(d++,f++,f===s&&(l.push(d-s),f=c[s-1])):f===0?d++:f=c[f-1];return{matched:l.length>0,matchIndexes:l,comparisonCount:u,executionTime:performance.now()-r,algorithm:`KMP`}}function _e(e){let t=new Map;for(let n=0;n<e.length;n++)t.set(e[n],n);return t}function ve(e,t,n=!1){let r=performance.now();if(t.length===0||e.length===0||t.length>e.length)return{matched:!1,matchIndexes:[],comparisonCount:0,executionTime:performance.now()-r,algorithm:`BM`};let i=n?e:e.toLowerCase(),a=n?t:t.toLowerCase(),o=0,s=_e(a),c=[],l=a.length,u=i.length,d=1;for(let e=0;e<=u-l;e+=d){d=0;for(let t=l-1;t>=0;t--){let n=i.charAt(e+t);if(o++,a.charAt(t)!=n){let e=t-(s.get(n)??-1);d=Math.max(1,e);break}}d===0&&(c.push(e),d=1)}return{matched:c.length>0,matchIndexes:c,comparisonCount:o,executionTime:performance.now()-r,algorithm:`BM`}}var ye=31,L=1000000007;function R(e){let t=e.charCodeAt(0);return t>=97&&t<=122?t-96:t>=48&&t<=57?t-48+27:t+37}function be(e){let t=0;for(let n=0;n<e.length;n++)t=(t*ye+R(e[n]))%L;return t}function xe(e){let t=1;for(let n=0;n<e-1;n++)t=t*ye%L;return t}function Se(e,t,n=!1){let r=performance.now();if(t.length===0||e.length===0||t.length>e.length)return{matched:!1,matchIndexes:[],matchLengths:[],comparisonCount:0,executionTime:performance.now()-r,algorithm:`RabinKarp`};let i=e,a=t;n||(i=e.toLowerCase(),a=t.toLowerCase());let o=i.length,s=a.length,c=be(a),l=xe(s),u=be(i.slice(0,s)),d=[],f=[],p=0;for(let e=0;e<=o-s;e++){if(p++,u===c){let t=!0;for(let n=0;n<s;n++)if(p++,i[e+n]!==a[n]){t=!1;break}t&&(d.push(e),f.push(s))}if(e<o-s){let t=R(i[e])*l%L;u=((u-t+L)*ye+R(i[e+s]))%L}}return{matched:d.length>0,matchIndexes:d,matchLengths:f,comparisonCount:p,executionTime:performance.now()-r,algorithm:`RabinKarp`}}var Ce=class{children;failureLink;outputs;constructor(){this.children=new Map,this.failureLink=null,this.outputs=[]}},we=class{root;constructor(e){this.root=new Ce,this.buildTrie(e),this.buildFailureLinks()}buildTrie(e){for(let t of e){let e=this.root;for(let n of t)e.children.has(n)||e.children.set(n,new Ce),e=e.children.get(n);e.outputs.push(t)}}buildFailureLinks(){let e=[];this.root.failureLink=this.root;for(let t of this.root.children.values())t.failureLink=this.root,e.push(t);for(;e.length>0;){let t=e.shift();for(let[n,r]of t.children){let i=t.failureLink;for(;i!==this.root&&!i.children.has(n);)i=i.failureLink;i.children.has(n)&&i.children.get(n)!==r?r.failureLink=i.children.get(n):r.failureLink=this.root,r.outputs.push(...r.failureLink.outputs),e.push(r)}}}processText(e){let t=new Map,n=0,r=this.root;for(let i=0;i<e.length;i++){let a=e[i];for(n++;r!==this.root&&!r.children.has(a);)r=r.failureLink;r.children.has(a)&&(r=r.children.get(a));for(let e of r.outputs){let n=i-e.length+1;t.has(e)||t.set(e,[]),t.get(e).push(n)}}return{matches:t,comparisonCount:n}}},Te=[[`0`,`o`,.1],[`1`,`l`,.1],[`1`,`i`,.1],[`3`,`e`,.2],[`4`,`a`,.2],[`5`,`s`,.2],[`7`,`t`,.25],[`8`,`b`,.3],[`9`,`g`,.1],[`@`,`a`,.1],[`$`,`s`,.1],[`!`,`i`,.2],[`!`,`l`,.2]],Ee=4,De=160,Oe=.7,ke=/\s/u,Ae=Object.create(null);for(let[e,t,n]of Te)je(e,t,n),je(t,e,n);function je(e,t,n){let r=Ae[e]??(Ae[e]=Object.create(null));r[t]=n}function Me(e,t){return e===t?0:Ae[e]?.[t]??1}function Ne(e){return{matched:!1,matchIndexes:[],matchLengths:[],comparisonCount:0,executionTime:performance.now()-e,algorithm:`Weighted Levenshtein`}}function Pe(e){return{previous:new Float64Array(e+1),current:new Float64Array(e+1)}}function Fe(e,t,n){return ke.test(e[t]??``)||ke.test(e[t+n-1]??``)}function Ie(e,t,n,r,i,a){let o=r.length,s=0,c=a.previous,l=a.current;c[0]=0;for(let e=1;e<=o;e++)c[e]=e;for(let a=1;a<=n;a++){l[0]=a;let n=l[0],u=e[t+a-1];for(let e=1;e<=o;e++){s++;let t=c[e]+1,i=l[e-1]+1,a=c[e-1]+Me(u,r[e-1]),o=t<i?t:i;a<o&&(o=a),l[e]=o,o<n&&(n=o)}if(n>i)return{distance:n,comparisonCount:s};let d=c;c=l,l=d}return{distance:c[o],comparisonCount:s}}function Le(e,t){let n=performance.now();if(e.length<Ee||t.length===0||e.length>t.length||t.length>De)return Ne(n);let r=[],i=[],a=0,o=e.length,s=t.toLowerCase(),c=e.toLowerCase(),l=(1-Oe)*o,u=Pe(o);for(let e=0;e<=s.length-o;e++){if(Fe(s,e,o))continue;let t=Ie(s,e,o,c,l,u);a+=t.comparisonCount,t.distance<=l&&(r.push(e),i.push(o))}return{matched:r.length>0,matchIndexes:r,matchLengths:i,comparisonCount:a,executionTime:performance.now()-n,algorithm:`Weighted Levenshtein`}}var Re=`0OoΟοσОоՕօסه٥ھہە۵߀०০੦૦ଠ୦௦ం౦ಂ೦ംഠ൦ං๐໐ဝ၀ჿዐᴏᴑℴⲞⲟⵔ〇ꓳꬽﮦﮧﮨﮩﮪﮫﮬﮭﻩﻪﻫﻬ０Ｏｏ𐊒𐊫𐐄𐐬𐓂𐓪𐔖𑓐𑢵𑣈𑣗𑣠𝐎𝐨𝑂𝑜𝑶𝒐𝒪𝓞𝓸𝔒𝔬𝕆𝕠𝕺𝖔𝖮𝗈𝗢𝗼𝘖𝘰𝙊𝙤𝙾𝚘𝚶𝛐𝛔𝛰𝜊𝜎𝜪𝝄𝝈𝝤𝝾𝞂𝞞𝞸𝞼𝟎𝟘𝟢𝟬𝟶𞸤𞹤𞺄🯰
1Iil|ıƖǀɩɪ˛ͺΙιІіӀӏ׀וןا١۱ߊᎥᛁιℐℑℓℹⅈⅠⅰⅼ∣⍳⏽Ⲓⵏꓲꙇꭵﺍﺎ１Ｉｉｌ￨𐊊𐌉𐌠𑣃𖼨𝐈𝐢𝐥𝐼𝑖𝑙𝑰𝒊𝒍𝒾𝓁𝓘𝓲𝓵𝔦𝔩𝕀𝕚𝕝𝕴𝖎𝖑𝖨𝗂𝗅𝗜𝗶𝗹𝘐𝘪𝘭𝙄𝙞𝙡𝙸𝚒𝚕𝚤𝚰𝛊𝛪𝜄𝜤𝜾𝝞𝝸𝞘𝞲𝟏𝟙𝟣𝟭𝟷𞣇𞸀𞺀🯱
2ƧϨᒿꙄꛯꝚ２𝟐𝟚𝟤𝟮𝟸🯲
3ƷȜЗӠⳌꝪꞫ３𑣊𖼻𝈆𝟑𝟛𝟥𝟯𝟹🯳
4Ꮞ４𑢯𝟒𝟜𝟦𝟰𝟺🯴
5Ƽ５𑢻𝟓𝟝𝟧𝟱𝟻🯵
6бᏮⳒ６𑣕𝟔𝟞𝟨𝟲𝟼🯶
7７𐓒𑣆𝈒𝟕𝟟𝟩𝟳𝟽🯷
8Ȣȣ৪੪ଃ８𐌚𝟖𝟠𝟪𝟴𝟾𞣋🯸
9৭੧୨൭ⳊꝮ９𑢬𑣌𑣖𝟗𝟡𝟫𝟵𝟿🯹
AΑАᎪᗅᴀꓮꭺＡ𐊠𖽀𝐀𝐴𝑨𝒜𝓐𝔄𝔸𝕬𝖠𝗔𝘈𝘼𝙰𝚨𝛢𝜜𝝖𝞐
BʙΒВвᏴᏼᗷᛒℬꓐꞴＢ𐊂𐊡𐌁𝐁𝐵𝑩𝓑𝔅𝔹𝕭𝖡𝗕𝘉𝘽𝙱𝚩𝛣𝜝𝝗𝞑
CϹСᏟᑕℂℭⅭ⊂Ⲥ⸦ꓚＣ𐊢𐌂𐐕𐔜𑣩𑣲𝐂𝐶𝑪𝒞𝓒𝕮𝖢𝗖𝘊𝘾𝙲🝌
DᎠᗞᗪᴅⅅⅮꓓꭰＤ𝐃𝐷𝑫𝒟𝓓𝔇𝔻𝕯𝖣𝗗𝘋𝘿𝙳
EΕЕᎬᴇℰ⋿ⴹꓰꭼＥ𐊆𑢦𑢮𝐄𝐸𝑬𝓔𝔈𝔼𝕰𝖤𝗘𝘌𝙀𝙴𝚬𝛦𝜠𝝚𝞔
FϜᖴℱꓝꞘＦ𐊇𐊥𐔥𑢢𑣂𝈓𝐅𝐹𝑭𝓕𝔉𝔽𝕱𝖥𝗙𝘍𝙁𝙵𝟊
GɢԌԍᏀᏳᏻꓖꮐＧ𝐆𝐺𝑮𝒢𝓖𝔊𝔾𝕲𝖦𝗚𝘎𝙂𝙶
HʜΗНнᎻᕼℋℌℍⲎꓧꮋＨ𐋏𝐇𝐻𝑯𝓗𝕳𝖧𝗛𝘏𝙃𝙷𝚮𝛨𝜢𝝜𝞖
JͿЈᎫᒍᴊꓙꞲꭻＪ𝐉𝐽𝑱𝒥𝓙𝔍𝕁𝕵𝖩𝗝𝘑𝙅𝙹
KΚКᏦᛕKⲔꓗＫ𐔘𝐊𝐾𝑲𝒦𝓚𝔎𝕂𝕶𝖪𝗞𝘒𝙆𝙺𝚱𝛫𝜥𝝟𝞙
LʟᏞᒪℒⅬⳐⳑꓡꮮＬ𐐛𐑃𐔦𑢣𑢲𖼖𝈪𝐋𝐿𝑳𝓛𝔏𝕃𝕷𝖫𝗟𝘓𝙇𝙻
MΜϺМᎷᗰᛖℳⅯⲘꓟＭ𐊰𐌑𝐌𝑀𝑴𝓜𝔐𝕄𝕸𝖬𝗠𝘔𝙈𝙼𝚳𝛭𝜧𝝡𝞛
NɴΝℕⲚꓠＮ𐔓𝐍𝑁𝑵𝒩𝓝𝔑𝕹𝖭𝗡𝘕𝙉𝙽𝚴𝛮𝜨𝝢𝞜
PΡРᏢᑭᴘᴩℙⲢꓑꮲＰ𐊕𝐏𝑃𝑷𝒫𝓟𝔓𝕻𝖯𝗣𝘗𝙋𝙿𝚸𝛲𝜬𝝦𝞠
QℚⵕＱ𝐐𝑄𝑸𝒬𝓠𝔔𝕼𝖰𝗤𝘘𝙌𝚀
RƦʀᎡᏒᖇᚱℛℜℝꓣꭱꮢＲ𐒴𖼵𝈖𝐑𝑅𝑹𝓡𝕽𝖱𝗥𝘙𝙍𝚁
SЅՏᏕᏚꓢＳ𐊖𐐠𖼺𝐒𝑆𝑺𝒮𝓢𝔖𝕊𝕾𝖲𝗦𝘚𝙎𝚂
TΤτТтᎢᴛ⊤⟙ⲦꓔꭲＴ𐊗𐊱𐌕𑢼𖼊𝐓𝑇𝑻𝒯𝓣𝔗𝕋𝕿𝖳𝗧𝘛𝙏𝚃𝚻𝛕𝛵𝜏𝜯𝝉𝝩𝞃𝞣𝞽🝨
UՍሀᑌ∪⋃ꓴＵ𐓎𑢸𖽂𝐔𝑈𝑼𝒰𝓤𝔘𝕌𝖀𝖴𝗨𝘜𝙐𝚄
VѴ٧۷ᏙᐯⅤⴸꓦꛟＶ𐔝𑢠𖼈𝈍𝐕𝑉𝑽𝒱𝓥𝔙𝕍𝖁𝖵𝗩𝘝𝙑𝚅
WԜᎳᏔꓪＷ𑣦𑣯𝐖𝑊𝑾𝒲𝓦𝔚𝕎𝖂𝖶𝗪𝘞𝙒𝚆
XΧХ᙭ᚷⅩ╳ⲬⵝꓫꞳＸ𐊐𐊴𐌗𐌢𐔧𑣬𝐗𝑋𝑿𝒳𝓧𝔛𝕏𝖃𝖷𝗫𝘟𝙓𝚇𝚾𝛸𝜲𝝬𝞦
YΥϒУҮᎩᎽⲨꓬＹ𐊲𑢤𖽃𝐘𝑌𝒀𝒴𝓨𝔜𝕐𝖄𝖸𝗬𝘠𝙔𝚈𝚼𝛶𝜰𝝪𝞤
ZΖᏃℤℨꓜＺ𐋵𑢩𑣥𝐙𝑍𝒁𝒵𝓩𝖅𝖹𝗭𝘡𝙕𝚉𝚭𝛧𝜡𝝛𝞕
aɑαа⍺ａ𝐚𝑎𝒂𝒶𝓪𝔞𝕒𝖆𝖺𝗮𝘢𝙖𝚊𝛂𝛼𝜶𝝰𝞪
bƄЬᏏᑲᖯｂ𝐛𝑏𝒃𝒷𝓫𝔟𝕓𝖇𝖻𝗯𝘣𝙗𝚋
cϲсᴄⅽⲥꮯｃ𐐽𝐜𝑐𝒄𝒸𝓬𝔠𝕔𝖈𝖼𝗰𝘤𝙘𝚌
dԁᏧᑯⅆⅾꓒｄ𝐝𝑑𝒅𝒹𝓭𝔡𝕕𝖉𝖽𝗱𝘥𝙙𝚍
eеҽ℮ℯⅇꬲｅ𝐞𝑒𝒆𝓮𝔢𝕖𝖊𝖾𝗲𝘦𝙚𝚎
fſϝքẝꞙꬵｆ𝐟𝑓𝒇𝒻𝓯𝔣𝕗𝖋𝖿𝗳𝘧𝙛𝚏𝟋
gƍɡցᶃℊｇ𝐠𝑔𝒈𝓰𝔤𝕘𝖌𝗀𝗴𝘨𝙜𝚐
hһհᏂℎｈ𝐡𝒉𝒽𝓱𝔥𝕙𝖍𝗁𝗵𝘩𝙝𝚑
jϳјⅉｊ𝐣𝑗𝒋𝒿𝓳𝔧𝕛𝖏𝗃𝗷𝘫𝙟𝚓
kｋ𝐤𝑘𝒌𝓀𝓴𝔨𝕜𝖐𝗄𝗸𝘬𝙠𝚔
mｍ
nոռｎ𝐧𝑛𝒏𝓃𝓷𝔫𝕟𝖓𝗇𝗻𝘯𝙣𝚗
pρϱр⍴ⲣｐ𝐩𝑝𝒑𝓅𝓹𝔭𝕡𝖕𝗉𝗽𝘱𝙥𝚙𝛒𝛠𝜌𝜚𝝆𝝔𝞀𝞎𝞺𝟈
qԛգզｑ𝐪𝑞𝒒𝓆𝓺𝔮𝕢𝖖𝗊𝗾𝘲𝙦𝚚
rгᴦⲅꭇꭈꮁｒ𝐫𝑟𝒓𝓇𝓻𝔯𝕣𝖗𝗋𝗿𝘳𝙧𝚛
sƽѕꜱꮪｓ𐑈𑣁𝐬𝑠𝒔𝓈𝓼𝔰𝕤𝖘𝗌𝘀𝘴𝙨𝚜
tｔ𝐭𝑡𝒕𝓉𝓽𝔱𝕥𝖙𝗍𝘁𝘵𝙩𝚝
uʋυսᴜꞟꭎꭒｕ𐓶𑣘𝐮𝑢𝒖𝓊𝓾𝔲𝕦𝖚𝗎𝘂𝘶𝙪𝚞𝛖𝜐𝝊𝞄𝞾
vνѵטᴠⅴ∨⋁ꮩｖ𑜆𑣀𝐯𝑣𝒗𝓋𝓿𝔳𝕧𝖛𝗏𝘃𝘷𝙫𝚟𝛎𝜈𝝂𝝼𝞶
wɯѡԝաᴡꮃｗ𑜊𑜎𑜏𝐰𝑤𝒘𝓌𝔀𝔴𝕨𝖜𝗐𝘄𝘸𝙬𝚠
x×хᕁᕽ᙮ⅹ⤫⤬⨯ｘ𝐱𝑥𝒙𝓍𝔁𝔵𝕩𝖝𝗑𝘅𝘹𝙭𝚡
yɣʏγуүყᶌỿℽꭚｙ𑣜𝐲𝑦𝒚𝓎𝔂𝔶𝕪𝖞𝗒𝘆𝘺𝙮𝚢𝛄𝛾𝜸𝝲𝞬
zᴢꮓｚ𑣄𝐳𝑧𝒛𝓏𝔃𝔷𝕫𝖟𝗓𝘇𝘻𝙯𝚣`,ze=/^[A-Za-z]$/u,Be=/^[a-z]$/u,Ve=/^[\x00-\x7F]$/u,He={ı:`i`,ɩ:`i`,ɪ:`i`,Ι:`i`,ι:`i`,І:`i`,і:`i`,Ӏ:`i`,Ɩ:`l`,ǀ:`l`,ӏ:`l`,"∣":`l`},Ue={...We(Re),...He};function We(e){let t={};return e.split(/\r?\n/u).map(e=>Array.from(e.trim())).filter(e=>e.length>0).forEach(e=>{let n=Ge(e);n!==void 0&&e.forEach(e=>{let r=e.normalize(`NFKD`).toLowerCase();Ke(r,n)||(t[r]=n)})}),t}function Ge(e){let t=e[0];if(t!==void 0&&ze.test(t))return t.toLowerCase();let n=new Set;if(e.forEach(e=>{let t=e.toLowerCase();Be.test(t)&&n.add(t)}),n.size===1)return[...n][0];if(n.has(`o`))return`o`;if(n.has(`i`))return`i`}function Ke(e,t){return e===t||Ve.test(e)?!0:e.length!==1}var qe=/\p{M}/u;function Je(e){let t=``,n=[],r=0;for(;r<e.length;){let i=e.codePointAt(r);if(i===void 0)break;let a=String.fromCodePoint(i),o=r+a.length,s=Xe(a),c=0;for(;c<s.length;){let e=s.codePointAt(c);if(e===void 0)break;let i=String.fromCodePoint(e);t+=i,n.push({start:r,end:o}),c+=i.length}r=o}return{value:t,map:n}}function Ye(e){return Je(e).value}function Xe(e){let t=e.normalize(`NFKD`),n=``,r=0;for(;r<t.length;){let e=t.codePointAt(r);if(e===void 0)break;let i=String.fromCodePoint(e);if(r+=i.length,qe.test(i))continue;let a=i.toLowerCase(),o=Ue[a];o===void 0?n+=a:n+=o}return n}var Ze=new we([...new Set(I.map(Ye))]);function Qe(e,t,n,r,i,a){let o=Ye(t),s=z(ge(n.value,o,!0),n.map,o.length),c=z(ve(n.value,o,!0),n.map,o.length),l=z(Se(n.value,o,!0),n.map,o.length),u=r.get(o)??[],d=z({matched:u.length>0,matchIndexes:u,matchLengths:u.map(()=>o.length),comparisonCount:i,executionTime:a,algorithm:`AhoCorasick`},n.map,o.length);return[s,c,l,d,s.matched||c.matched||l.matched||d.matched?{matched:!1,matchIndexes:[],matchLengths:[],comparisonCount:0,executionTime:0,algorithm:`Weighted Levenshtein`}:Le(t,e)]}function $e(e){let t=performance.now(),{matches:n,comparisonCount:r}=Ze.processText(e);return{matches:n,comparisonCount:r,elapsedMs:performance.now()-t}}function z(e,t,n){if(!e.matched)return e;let r=[],i=[];for(let a=0;a<e.matchIndexes.length;a++){let o=e.matchIndexes[a],s=et(t,o,e.matchLengths?.[a]??n);s!==null&&(r.push(s.start),i.push(s.end-s.start))}return{...e,matched:r.length>0,matchIndexes:r,matchLengths:i}}function et(e,t,n){let r=e[t],i=e[t+n-1];return r===void 0||i===void 0?null:{start:r.start,end:i.end}}var B={blurTextEnabled:!0};function tt(e){try{let t=V();if(t?.storage?.local?.set({[v]:e}),e.pageUrl){let n=`${v}::${e.pageUrl}`;t?.storage?.local?.set({[n]:e})}}catch{}}function nt(e){V()?.runtime?.onMessage?.addListener((t,n,r)=>t.type===`JUDOL_RESCAN_REQUEST`?(e(),r({ok:!0}),!0):!1)}function rt(e){try{let t=V();if(!t?.storage?.local?.get){e(B);return}t.storage.local.get({[y]:B},t=>{e(at(t[y]))})}catch{e(B)}}function it(e){V()?.storage?.onChanged?.addListener((t,n)=>{if(n!==`local`)return;let r=t[y];r&&e(at(r.newValue))})}function at(e){let t=e;return{blurTextEnabled:typeof t?.blurTextEnabled==`boolean`?t.blurTextEnabled:B.blurTextEnabled}}function V(){return globalThis.chrome}var ot=[`KMP`,`BM`,`RegEx`,`Weighted Levenshtein`,`AhoCorasick`,`RabinKarp`];function st(e){let t={};for(let e of ot)t[e]={algorithm:e,matches:0,executionTime:0,comparisonCount:0};return{scanId:e,scannedAt:Date.now(),pageUrl:window.location.href,pageTitle:document.title||window.location.hostname||`Untitled page`,totalKeywordsFound:0,totalMatches:0,algorithms:t,keywordFrequency:[]}}function ct(e,t,n,r,i={}){let a=e.algorithms[t.algorithm];a.matches+=t.matchIndexes.length,(i.countCost??!0)&&(a.executionTime+=t.executionTime,a.comparisonCount+=t.comparisonCount);let o=pt(n,t.algorithm);r.set(o,(r.get(o)??0)+t.executionTime)}function H(e,t,n,r){let i=e.algorithms[t];i.executionTime+=n,i.comparisonCount+=r}function lt(e,t,n){e.algorithms[t].matches+=n}function ut(e,t){let n=new Map;for(let e of t)n.set(e.keyword,(n.get(e.keyword)??0)+1);e.totalMatches=t.length,e.totalKeywordsFound=n.size,e.keywordFrequency=Array.from(n.entries()).map(([e,t])=>({keyword:e,occurrences:t})).sort((e,t)=>t.occurrences-e.occurrences||e.keyword.localeCompare(t.keyword))}function dt(e,t){let n=new Map;for(let t of e)n.set(t.keyword,(n.get(t.keyword)??0)+1);for(let r of e)r.occurrences=n.get(r.keyword)??1,r.executionTime=t.get(pt(r.keyword,r.algorithm))??r.executionTime}function ft(e,t,n){e.totalMatches+=n;let r=e.keywordFrequency.find(e=>e.keyword===t);r?r.occurrences+=n:(e.totalKeywordsFound++,e.keywordFrequency.push({keyword:t,occurrences:n})),e.keywordFrequency.sort((e,t)=>t.occurrences-e.occurrences||e.keyword.localeCompare(t.keyword))}function pt(e,t){return`${t}::${e}`}var mt=!1,U=null;function ht(){Dt(),!mt&&(document.addEventListener(`mouseover`,_t,!0),document.addEventListener(`mouseout`,vt,!0),document.addEventListener(`focusin`,yt,!0),document.addEventListener(`focusout`,bt,!0),window.addEventListener(`scroll`,Ct,!0),window.addEventListener(`resize`,Ct,!0),mt=!0)}function gt(){let e=Array.from(document.querySelectorAll(`.${f}`));for(let t of e)t.remove();U=null}function _t(e){let t=Tt(e.target);t&&xt(t)}function vt(e){U&&(e.relatedTarget instanceof Node&&U.contains(e.relatedTarget)||St())}function yt(e){let t=Tt(e.target);t&&xt(t)}function bt(){St()}function xt(e){U=e;let t=wt(),n=Et(e);t.replaceChildren(W(`Keyword`,n.keyword),W(`Algorithm`,n.algorithm),W(`Occurrences`,String(n.occurrences)),W(`Execution`,`${n.executionTime.toFixed(3)} ms`)),t.hidden=!1,Ct()}function St(){let e=document.getElementById(d);e&&(e.hidden=!0),U=null}function Ct(){if(!U)return;let e=document.getElementById(d);if(!(e instanceof HTMLElement)||e.hidden)return;let t=U.getBoundingClientRect(),n=e.getBoundingClientRect(),r=t.top-n.height-8,i=r>=8?r:t.bottom+8,a=t.left+t.width/2-n.width/2,o=window.innerWidth-n.width-8,s=Math.min(Math.max(8,a),Math.max(8,o));e.style.transform=`translate(${Math.round(s)}px, ${Math.round(i)}px)`}function wt(){let e=document.getElementById(d);if(e instanceof HTMLElement)return e;let t=document.createElement(`div`);return t.id=d,t.className=f,t.hidden=!0,document.body.appendChild(t),t}function W(e,t){let n=document.createElement(`div`),r=document.createElement(`span`),i=document.createElement(`strong`);return n.className=m,r.textContent=e,i.textContent=t,n.append(r,i),n}function Tt(e){return e instanceof Element?e.closest(`.${u}`)||e.closest(`img.${g}`)||e.closest(`.${_}`):null}function Et(e){return{keyword:e.dataset.keyword??`-`,algorithm:e.dataset.algorithm??`KMP`,occurrences:Number(e.dataset.occurrences??`0`),executionTime:Number(e.dataset.executionTime??`0`)}}function Dt(){if(document.getElementById(`judol-tooltip-style`))return;let e=document.createElement(`style`);e.id=p,e.textContent=`
        .${f} {
            background: #171717;
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 6px;
            box-shadow: 0 14px 34px rgba(0, 0, 0, 0.28);
            color: #ffffff;
            font: 12px/1.4 system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
            left: 0;
            max-width: min(280px, calc(100vw - 16px));
            padding: 10px 12px;
            pointer-events: none;
            position: fixed;
            top: 0;
            z-index: 2147483647;
        }

        .${f}[hidden] {
            display: none;
        }

        .${m} {
            display: grid;
            gap: 14px;
            grid-template-columns: max-content minmax(0, 1fr);
            min-width: 190px;
        }

        .${m} + .${m} {
            margin-top: 5px;
        }

        .${m} span {
            color: #bdbdbd;
        }

        .${m} strong {
            color: #ffffff;
            font-weight: 650;
            overflow-wrap: anywhere;
            text-align: right;
        }
    `,document.head.appendChild(e)}var Ot=50,kt=new Set([`NOSCRIPT`,`TEMPLATE`]);function At(e){let t=[];for(let n of Array.from(e.querySelectorAll(`img`)))Pt(n)&&t.push(n);return t}function G(e){return`${e.currentSrc||e.src}|${e.naturalWidth}x${e.naturalHeight}`}function jt(e,t=G(e)){e.dataset.judolDetected=`true`,e.dataset.judolImageKey=t}function Mt(e){return e.classList.contains(`judol-image`)&&e.dataset.judolDetected===`true`&&e.dataset.judolImageKey===G(e)}function Nt(e){delete e.dataset.judolDetected,delete e.dataset.judolImageKey}function Pt(e){if(Mt(e)||e.classList.contains(`judol-image`)&&e.dataset.judolImageKey===G(e))return!1;let t=e.parentElement;return!(t!==null&&kt.has(t.tagName)||!e.src&&!e.currentSrc||e.naturalWidth>0&&e.naturalWidth<Ot||e.naturalHeight>0&&e.naturalHeight<Ot)}var Ft=null;function It(e){Ft=e}function Lt(){return Ft}function Rt(e,t,n,r){let{imageElement:i,extractedText:a}=e;if(!a.trim())return!1;let o=null,s=`KMP`,c=0,l=0,u=[],d=Je(a),f=$e(d.value);H(n,`AhoCorasick`,f.elapsedMs,f.comparisonCount);let p=D(a,d);H(n,`RegEx`,p.executionMs,a.length),lt(n,`RegEx`,p.matches.length),l+=Bt(u,p.matches),p.matches.length>0&&(o=p.matches[0].keyword,s=`RegEx`,c=p.executionMs);for(let e of t){let t=Qe(a,e,d,f.matches,f.comparisonCount,f.elapsedMs);for(let i of t)ct(n,i,e,r,{countCost:!zt(i.algorithm)}),i.matched&&i.matchIndexes.length>0&&(l+=Vt(u,i,e),o===null&&(o=e,s=i.algorithm,c=i.executionTime))}return o===null?!1:(Ut(i,{keyword:o,algorithm:s,occurrences:l,executionTime:c}),ft(n,o,l),!0)}function zt(e){return e===`AhoCorasick`}function Bt(e,t){let n=0;for(let r of t)Ht(e,r.start,r.end)&&n++;return n}function Vt(e,t,n){let r=0;for(let i=0;i<t.matchIndexes.length;i++){let a=t.matchIndexes[i];Ht(e,a,a+(t.matchLengths?.[i]??n.length))&&r++}return r}function Ht(e,t,n){return e.some(e=>t<e.end&&e.start<n)?!1:(e.push({start:t,end:n}),!0)}function Ut(e,t){e.classList.contains(`judol-image`)||(e.dataset.originalFilter===void 0&&(e.dataset.originalFilter=e.style.filter),e.dataset.judolAction=`blur`,Gt(e,t),e.classList.add(g),jt(e))}function Wt(e){e.dataset.judolAction===`blur`&&(e.style.filter=e.dataset.originalFilter??``,delete e.dataset.originalFilter),e.classList.remove(g),delete e.dataset.keyword,delete e.dataset.algorithm,delete e.dataset.occurrences,delete e.dataset.executionTime,delete e.dataset.judolAction,Nt(e)}function Gt(e,t){e.dataset.keyword=t.keyword,e.dataset.algorithm=t.algorithm,e.dataset.occurrences=String(t.occurrences),e.dataset.executionTime=t.executionTime.toFixed(3)}var Kt=a(((e,t)=>{var n=function(e){var t=Object.prototype,n=t.hasOwnProperty,r=Object.defineProperty||function(e,t,n){e[t]=n.value},i,a=typeof Symbol==`function`?Symbol:{},o=a.iterator||`@@iterator`,s=a.asyncIterator||`@@asyncIterator`,c=a.toStringTag||`@@toStringTag`;function l(e,t,n){return Object.defineProperty(e,t,{value:n,enumerable:!0,configurable:!0,writable:!0}),e[t]}try{l({},``)}catch{l=function(e,t,n){return e[t]=n}}function u(e,t,n,i){var a=t&&t.prototype instanceof _?t:_,o=Object.create(a.prototype);return r(o,`_invoke`,{value:E(e,n,new k(i||[]))}),o}e.wrap=u;function d(e,t,n){try{return{type:`normal`,arg:e.call(t,n)}}catch(e){return{type:`throw`,arg:e}}}var f=`suspendedStart`,p=`suspendedYield`,m=`executing`,h=`completed`,g={};function _(){}function v(){}function y(){}var b={};l(b,o,function(){return this});var x=Object.getPrototypeOf,S=x&&x(x(A([])));S&&S!==t&&n.call(S,o)&&(b=S);var C=y.prototype=_.prototype=Object.create(b);v.prototype=y,r(C,`constructor`,{value:y,configurable:!0}),r(y,`constructor`,{value:v,configurable:!0}),v.displayName=l(y,c,`GeneratorFunction`);function w(e){[`next`,`throw`,`return`].forEach(function(t){l(e,t,function(e){return this._invoke(t,e)})})}e.isGeneratorFunction=function(e){var t=typeof e==`function`&&e.constructor;return t?t===v||(t.displayName||t.name)===`GeneratorFunction`:!1},e.mark=function(e){return Object.setPrototypeOf?Object.setPrototypeOf(e,y):(e.__proto__=y,l(e,c,`GeneratorFunction`)),e.prototype=Object.create(C),e},e.awrap=function(e){return{__await:e}};function T(e,t){function i(r,a,o,s){var c=d(e[r],e,a);if(c.type===`throw`)s(c.arg);else{var l=c.arg,u=l.value;return u&&typeof u==`object`&&n.call(u,`__await`)?t.resolve(u.__await).then(function(e){i(`next`,e,o,s)},function(e){i(`throw`,e,o,s)}):t.resolve(u).then(function(e){l.value=e,o(l)},function(e){return i(`throw`,e,o,s)})}}var a;function o(e,n){function r(){return new t(function(t,r){i(e,n,t,r)})}return a=a?a.then(r,r):r()}r(this,`_invoke`,{value:o})}w(T.prototype),l(T.prototype,s,function(){return this}),e.AsyncIterator=T,e.async=function(t,n,r,i,a){a===void 0&&(a=Promise);var o=new T(u(t,n,r,i),a);return e.isGeneratorFunction(n)?o:o.next().then(function(e){return e.done?e.value:o.next()})};function E(e,t,n){var r=f;return function(i,a){if(r===m)throw Error(`Generator is already running`);if(r===h){if(i===`throw`)throw a;return j()}for(n.method=i,n.arg=a;;){var o=n.delegate;if(o){var s=D(o,n);if(s){if(s===g)continue;return s}}if(n.method===`next`)n.sent=n._sent=n.arg;else if(n.method===`throw`){if(r===f)throw r=h,n.arg;n.dispatchException(n.arg)}else n.method===`return`&&n.abrupt(`return`,n.arg);r=m;var c=d(e,t,n);if(c.type===`normal`){if(r=n.done?h:p,c.arg===g)continue;return{value:c.arg,done:n.done}}else c.type===`throw`&&(r=h,n.method=`throw`,n.arg=c.arg)}}}function D(e,t){var n=t.method,r=e.iterator[n];if(r===i)return t.delegate=null,n===`throw`&&e.iterator.return&&(t.method=`return`,t.arg=i,D(e,t),t.method===`throw`)||n!==`return`&&(t.method=`throw`,t.arg=TypeError(`The iterator does not provide a '`+n+`' method`)),g;var a=d(r,e.iterator,t.arg);if(a.type===`throw`)return t.method=`throw`,t.arg=a.arg,t.delegate=null,g;var o=a.arg;if(!o)return t.method=`throw`,t.arg=TypeError(`iterator result is not an object`),t.delegate=null,g;if(o.done)t[e.resultName]=o.value,t.next=e.nextLoc,t.method!==`return`&&(t.method=`next`,t.arg=i);else return o;return t.delegate=null,g}w(C),l(C,c,`Generator`),l(C,o,function(){return this}),l(C,`toString`,function(){return`[object Generator]`});function ee(e){var t={tryLoc:e[0]};1 in e&&(t.catchLoc=e[1]),2 in e&&(t.finallyLoc=e[2],t.afterLoc=e[3]),this.tryEntries.push(t)}function O(e){var t=e.completion||{};t.type=`normal`,delete t.arg,e.completion=t}function k(e){this.tryEntries=[{tryLoc:`root`}],e.forEach(ee,this),this.reset(!0)}e.keys=function(e){var t=Object(e),n=[];for(var r in t)n.push(r);return n.reverse(),function e(){for(;n.length;){var r=n.pop();if(r in t)return e.value=r,e.done=!1,e}return e.done=!0,e}};function A(e){if(e){var t=e[o];if(t)return t.call(e);if(typeof e.next==`function`)return e;if(!isNaN(e.length)){var r=-1,a=function t(){for(;++r<e.length;)if(n.call(e,r))return t.value=e[r],t.done=!1,t;return t.value=i,t.done=!0,t};return a.next=a}}return{next:j}}e.values=A;function j(){return{value:i,done:!0}}return k.prototype={constructor:k,reset:function(e){if(this.prev=0,this.next=0,this.sent=this._sent=i,this.done=!1,this.delegate=null,this.method=`next`,this.arg=i,this.tryEntries.forEach(O),!e)for(var t in this)t.charAt(0)===`t`&&n.call(this,t)&&!isNaN(+t.slice(1))&&(this[t]=i)},stop:function(){this.done=!0;var e=this.tryEntries[0].completion;if(e.type===`throw`)throw e.arg;return this.rval},dispatchException:function(e){if(this.done)throw e;var t=this;function r(n,r){return s.type=`throw`,s.arg=e,t.next=n,r&&(t.method=`next`,t.arg=i),!!r}for(var a=this.tryEntries.length-1;a>=0;--a){var o=this.tryEntries[a],s=o.completion;if(o.tryLoc===`root`)return r(`end`);if(o.tryLoc<=this.prev){var c=n.call(o,`catchLoc`),l=n.call(o,`finallyLoc`);if(c&&l){if(this.prev<o.catchLoc)return r(o.catchLoc,!0);if(this.prev<o.finallyLoc)return r(o.finallyLoc)}else if(c){if(this.prev<o.catchLoc)return r(o.catchLoc,!0)}else if(l){if(this.prev<o.finallyLoc)return r(o.finallyLoc)}else throw Error(`try statement without catch or finally`)}}},abrupt:function(e,t){for(var r=this.tryEntries.length-1;r>=0;--r){var i=this.tryEntries[r];if(i.tryLoc<=this.prev&&n.call(i,`finallyLoc`)&&this.prev<i.finallyLoc){var a=i;break}}a&&(e===`break`||e===`continue`)&&a.tryLoc<=t&&t<=a.finallyLoc&&(a=null);var o=a?a.completion:{};return o.type=e,o.arg=t,a?(this.method=`next`,this.next=a.finallyLoc,g):this.complete(o)},complete:function(e,t){if(e.type===`throw`)throw e.arg;return e.type===`break`||e.type===`continue`?this.next=e.arg:e.type===`return`?(this.rval=this.arg=e.arg,this.method=`return`,this.next=`end`):e.type===`normal`&&t&&(this.next=t),g},finish:function(e){for(var t=this.tryEntries.length-1;t>=0;--t){var n=this.tryEntries[t];if(n.finallyLoc===e)return this.complete(n.completion,n.afterLoc),O(n),g}},catch:function(e){for(var t=this.tryEntries.length-1;t>=0;--t){var n=this.tryEntries[t];if(n.tryLoc===e){var r=n.completion;if(r.type===`throw`){var i=r.arg;O(n)}return i}}throw Error(`illegal catch attempt`)},delegateYield:function(e,t,n){return this.delegate={iterator:A(e),resultName:t,nextLoc:n},this.method===`next`&&(this.arg=i),g}},e}(typeof t==`object`?t.exports:{});try{regeneratorRuntime=n}catch{typeof globalThis==`object`?globalThis.regeneratorRuntime=n:Function(`r`,`regeneratorRuntime = r`)(n)}})),qt=a(((e,t)=>{t.exports=(e,t)=>`${e}-${t}-${Math.random().toString(16).slice(3,8)}`})),Jt=a(((e,t)=>{var n=qt(),r=0;t.exports=({id:e,action:t,payload:i={}})=>{let a=e;return a===void 0&&(a=n(`Job`,r),r+=1),{id:a,action:t,payload:i}}})),Yt=a((e=>{var t=!1;e.logging=t,e.setLogging=e=>{t=e},e.log=(...n)=>t?console.log.apply(e,n):null})),Xt=a(((e,t)=>{var n=Jt(),{log:r}=Yt(),i=qt(),a=0;t.exports=()=>{let t=i(`Scheduler`,a),o={},s={},c=[];a+=1;let l=()=>c.length,u=()=>Object.keys(o).length,d=()=>{if(c.length!==0){let e=Object.keys(o);for(let t=0;t<e.length;t+=1)if(s[e[t]]===void 0){c[0](o[e[t]]);break}}},f=(i,a)=>new Promise((o,l)=>{let u=n({action:i,payload:a});c.push(async t=>{c.shift(),s[t.id]=u;try{o(await t[i].apply(e,[...a,u.id]))}catch(e){l(e)}finally{delete s[t.id],d()}}),r(`[${t}]: Add ${u.id} to JobQueue`),r(`[${t}]: JobQueue length=${c.length}`),d()});return{addWorker:e=>(o[e.id]=e,r(`[${t}]: Add ${e.id}`),r(`[${t}]: Number of workers=${u()}`),d(),e.id),addJob:async(e,...n)=>{if(u()===0)throw Error(`[${t}]: You need to have at least one worker before adding jobs`);return f(e,n)},terminate:async()=>{Object.keys(o).forEach(async e=>{await o[e].terminate()}),c=[]},getQueueLen:l,getNumWorkers:u}}})),Zt=a(((e,t)=>{t.exports=e=>{let t={};return typeof WorkerGlobalScope<`u`?t.type=`webworker`:typeof document==`object`?t.type=`browser`:typeof process==`object`&&typeof l==`function`&&(t.type=`node`),e===void 0?t:t[e]}})),Qt=a(((e,t)=>{var n=Zt()(`type`)===`browser`?e=>new URL(e,window.location.href).href:e=>e;t.exports=e=>{let t={...e};return[`corePath`,`workerPath`,`langPath`].forEach(r=>{e[r]&&(t[r]=n(t[r]))}),t}})),$t=a(((e,t)=>{t.exports={TESSERACT_ONLY:0,LSTM_ONLY:1,TESSERACT_LSTM_COMBINED:2,DEFAULT:3}})),en=o({author:()=>``,browser:()=>dn,bugs:()=>vn,collective:()=>bn,contributors:()=>fn,default:()=>xn,dependencies:()=>hn,description:()=>rn,devDependencies:()=>mn,homepage:()=>yn,jsdelivr:()=>ln,license:()=>pn,main:()=>an,name:()=>tn,overrides:()=>gn,repository:()=>_n,scripts:()=>un,type:()=>on,types:()=>sn,unpkg:()=>cn,version:()=>nn}),tn,nn,rn,an,on,sn,cn,ln,un,dn,fn,pn,mn,hn,gn,_n,vn,yn,bn,xn,Sn=i((()=>{tn=`tesseract.js`,nn=`7.0.0`,rn=`Pure Javascript Multilingual OCR`,an=`src/index.js`,on=`commonjs`,sn=`src/index.d.ts`,cn=`dist/tesseract.min.js`,ln=`dist/tesseract.min.js`,un={start:`node scripts/server.js`,build:`rimraf dist && webpack --config scripts/webpack.config.prod.js && rollup -c scripts/rollup.esm.mjs`,"profile:tesseract":`webpack-bundle-analyzer dist/tesseract-stats.json`,"profile:worker":`webpack-bundle-analyzer dist/worker-stats.json`,prepublishOnly:`npm run build`,wait:`rimraf dist && wait-on http://localhost:3000/dist/tesseract.min.js`,test:`npm-run-all -p -r start test:all`,"test:all":`npm-run-all wait test:browser test:node:all`,"test:browser":`karma start karma.conf.js`,"test:node":`nyc mocha --exit --bail --require ./scripts/test-helper.mjs`,"test:node:all":`npm run test:node -- ./tests/*.test.mjs`,lint:`eslint src`,"lint:fix":`eslint --fix src`,postinstall:`opencollective-postinstall || true`},dn={"./src/worker/node/index.js":`./src/worker/browser/index.js`},fn=[`jeromewu`],pn=`Apache-2.0`,mn={"@babel/core":`^7.21.4`,"@babel/eslint-parser":`^7.21.3`,"@babel/preset-env":`^7.21.4`,"@rollup/plugin-commonjs":`^24.1.0`,acorn:`^8.8.2`,"babel-loader":`^9.1.2`,buffer:`^6.0.3`,cors:`^2.8.5`,eslint:`^7.32.0`,"eslint-config-airbnb-base":`^14.2.1`,"eslint-plugin-import":`^2.27.5`,"expect.js":`^0.3.1`,express:`^4.18.2`,mocha:`^10.2.0`,"npm-run-all":`^4.1.5`,karma:`^6.4.2`,"karma-chrome-launcher":`^3.2.0`,"karma-firefox-launcher":`^2.1.2`,"karma-mocha":`^2.0.1`,"karma-webpack":`^5.0.0`,nyc:`^15.1.0`,rimraf:`^5.0.0`,rollup:`^3.20.7`,"wait-on":`^7.0.1`,webpack:`^5.79.0`,"webpack-bundle-analyzer":`^4.8.0`,"webpack-cli":`^5.0.1`,"webpack-dev-middleware":`^6.0.2`,"rollup-plugin-sourcemaps":`^0.6.3`},hn={"bmp-js":`^0.1.0`,"idb-keyval":`^6.2.0`,"is-url":`^1.2.4`,"node-fetch":`^2.6.9`,"opencollective-postinstall":`^2.0.3`,"regenerator-runtime":`^0.13.3`,"tesseract.js-core":`^7.0.0`,"wasm-feature-detect":`^1.8.0`,zlibjs:`^0.3.1`},gn={"@rollup/pluginutils":`^5.0.2`},_n={type:`git`,url:`https://github.com/naptha/tesseract.js.git`},vn={url:`https://github.com/naptha/tesseract.js/issues`},yn=`https://github.com/naptha/tesseract.js`,bn={type:`opencollective`,url:`https://opencollective.com/tesseractjs`},xn={name:tn,version:nn,description:rn,main:an,type:on,types:sn,unpkg:cn,jsdelivr:ln,scripts:un,browser:dn,author:``,contributors:fn,license:pn,devDependencies:mn,dependencies:hn,overrides:gn,repository:_n,bugs:vn,homepage:yn,collective:bn}})),Cn=a(((e,t)=>{t.exports={workerBlobURL:!0,logger:()=>{}}})),wn=a(((e,t)=>{var n=(Sn(),c(en).default).version;t.exports={...Cn(),workerPath:`https://cdn.jsdelivr.net/npm/tesseract.js@v${n}/dist/worker.min.js`}})),Tn=a(((e,t)=>{t.exports=({workerPath:e,workerBlobURL:t})=>{let n;if(Blob&&URL&&t){let t=new Blob([`importScripts("${e}");`],{type:`application/javascript`});n=new Worker(URL.createObjectURL(t))}else n=new Worker(e);return n}})),En=a(((e,t)=>{t.exports=e=>{e.terminate()}})),Dn=a(((e,t)=>{t.exports=(e,t)=>{e.onmessage=({data:e})=>{t(e)}}})),On=a(((e,t)=>{t.exports=async(e,t)=>{e.postMessage(t)}})),kn=a(((e,t)=>{var n=e=>new Promise((t,n)=>{let r=new FileReader;r.onload=()=>{t(r.result)},r.onerror=({target:{error:{code:e}}})=>{n(Error(`File could not be read! Code=${e}`))},r.readAsArrayBuffer(e)}),r=async e=>{let t=e;return e===void 0?`undefined`:(typeof e==`string`?t=/data:image\/([a-zA-Z]*);base64,([^"]*)/.test(e)?atob(e.split(`,`)[1]).split(``).map(e=>e.charCodeAt(0)):await(await fetch(e)).arrayBuffer():typeof HTMLElement<`u`&&e instanceof HTMLElement?(e.tagName===`IMG`&&(t=await r(e.src)),e.tagName===`VIDEO`&&(t=await r(e.poster)),e.tagName===`CANVAS`&&await new Promise(r=>{e.toBlob(async e=>{t=await n(e),r()})})):typeof OffscreenCanvas<`u`&&e instanceof OffscreenCanvas?t=await n(await e.convertToBlob()):(e instanceof File||e instanceof Blob)&&(t=await n(e)),new Uint8Array(t))};t.exports=r})),An=a(((e,t)=>{t.exports={defaultOptions:wn(),spawnWorker:Tn(),terminateWorker:En(),onMessage:Dn(),send:On(),loadImage:kn()}})),jn=a(((e,t)=>{var n=Qt(),r=Jt(),{log:i}=Yt(),a=qt(),o=$t(),{defaultOptions:s,spawnWorker:c,terminateWorker:l,onMessage:u,loadImage:d,send:f}=An(),p=0;t.exports=async(e=`eng`,t=o.LSTM_ONLY,m={},h={})=>{let g=a(`Worker`,p),{logger:_,errorHandler:v,...y}=n({...s,...m}),b={},x=typeof e==`string`?e.split(`+`):e,S=t,C=h,w=[o.DEFAULT,o.LSTM_ONLY].includes(t)&&!y.legacyCore,T,E,D=new Promise((e,t)=>{E=e,T=t}),ee=e=>{T(e.message)},O=c(y);O.onerror=ee,p+=1;let k=({id:e,action:t,payload:n})=>new Promise((r,a)=>{i(`[${g}]: Start ${e}, action=${t}`);let o=`${t}-${e}`;b[o]={resolve:r,reject:a},f(O,{workerId:g,jobId:e,action:t,payload:n})}),A=()=>console.warn("`load` is depreciated and should be removed from code (workers now come pre-loaded)"),j=e=>k(r({id:e,action:`load`,payload:{options:{lstmOnly:w,corePath:y.corePath,logging:y.logging}}})),te=(e,t,n)=>k(r({id:n,action:`FS`,payload:{method:`writeFile`,args:[e,t]}})),M=(e,t)=>k(r({id:t,action:`FS`,payload:{method:`readFile`,args:[e,{encoding:`utf8`}]}})),ne=(e,t)=>k(r({id:t,action:`FS`,payload:{method:`unlink`,args:[e]}})),re=(e,t,n)=>k(r({id:n,action:`FS`,payload:{method:e,args:t}})),N=(e,t)=>k(r({id:t,action:`loadLanguage`,payload:{langs:e,options:{langPath:y.langPath,dataPath:y.dataPath,cachePath:y.cachePath,cacheMethod:y.cacheMethod,gzip:y.gzip,lstmOnly:[o.DEFAULT,o.LSTM_ONLY].includes(S)&&!y.legacyLang}}})),P=(e,t,n,i)=>k(r({id:i,action:`initialize`,payload:{langs:e,oem:t,config:n}})),ie=(e=`eng`,t,n,r)=>{if(w&&[o.TESSERACT_ONLY,o.TESSERACT_LSTM_COMBINED].includes(t))throw Error(`Legacy model requested but code missing.`);let i=t||S;S=i;let a=n||C;C=a;let s=(typeof e==`string`?e.split(`+`):e).filter(e=>!x.includes(e));return x.push(...s),s.length>0?N(s,r).then(()=>P(e,i,a,r)):P(e,i,a,r)},ae=(e={},t)=>k(r({id:t,action:`setParameters`,payload:{params:e}})),oe=async(e,t={},n={text:!0},i)=>k(r({id:i,action:`recognize`,payload:{image:await d(e),options:t,output:n}})),se=async(e,t)=>{if(w)throw Error("`worker.detect` requires Legacy model, which was not loaded.");return k(r({id:t,action:`detect`,payload:{image:await d(e)}}))},ce=async()=>(O!==null&&(l(O),O=null),Promise.resolve());u(O,({workerId:e,jobId:t,status:n,action:r,data:a})=>{let o=`${r}-${t}`;if(n===`resolve`)i(`[${e}]: Complete ${t}`),b[o].resolve({jobId:t,data:a}),delete b[o];else if(n===`reject`)if(b[o].reject(a),delete b[o],r===`load`&&T(a),v)v(a);else throw Error(a);else n===`progress`&&_({...a,userJobId:t})});let le={id:g,worker:O,load:A,writeText:te,readText:M,removeFile:ne,FS:re,reinitialize:ie,setParameters:ae,recognize:oe,detect:se,terminate:ce};return j().then(()=>N(e)).then(()=>P(e,t,h)).then(()=>E(le)).catch(()=>{}),D}})),Mn=a(((e,t)=>{var n=jn();t.exports={recognize:async(e,t,r)=>{let i=await n(t,1,r);return i.recognize(e).finally(async()=>{await i.terminate()})},detect:async(e,t)=>{let r=await n(`osd`,0,t);return r.detect(e).finally(async()=>{await r.terminate()})}}})),Nn=a(((e,t)=>{t.exports={AFR:`afr`,AMH:`amh`,ARA:`ara`,ASM:`asm`,AZE:`aze`,AZE_CYRL:`aze_cyrl`,BEL:`bel`,BEN:`ben`,BOD:`bod`,BOS:`bos`,BUL:`bul`,CAT:`cat`,CEB:`ceb`,CES:`ces`,CHI_SIM:`chi_sim`,CHI_TRA:`chi_tra`,CHR:`chr`,CYM:`cym`,DAN:`dan`,DEU:`deu`,DZO:`dzo`,ELL:`ell`,ENG:`eng`,ENM:`enm`,EPO:`epo`,EST:`est`,EUS:`eus`,FAS:`fas`,FIN:`fin`,FRA:`fra`,FRK:`frk`,FRM:`frm`,GLE:`gle`,GLG:`glg`,GRC:`grc`,GUJ:`guj`,HAT:`hat`,HEB:`heb`,HIN:`hin`,HRV:`hrv`,HUN:`hun`,IKU:`iku`,IND:`ind`,ISL:`isl`,ITA:`ita`,ITA_OLD:`ita_old`,JAV:`jav`,JPN:`jpn`,KAN:`kan`,KAT:`kat`,KAT_OLD:`kat_old`,KAZ:`kaz`,KHM:`khm`,KIR:`kir`,KOR:`kor`,KUR:`kur`,LAO:`lao`,LAT:`lat`,LAV:`lav`,LIT:`lit`,MAL:`mal`,MAR:`mar`,MKD:`mkd`,MLT:`mlt`,MSA:`msa`,MYA:`mya`,NEP:`nep`,NLD:`nld`,NOR:`nor`,ORI:`ori`,PAN:`pan`,POL:`pol`,POR:`por`,PUS:`pus`,RON:`ron`,RUS:`rus`,SAN:`san`,SIN:`sin`,SLK:`slk`,SLV:`slv`,SPA:`spa`,SPA_OLD:`spa_old`,SQI:`sqi`,SRP:`srp`,SRP_LATN:`srp_latn`,SWA:`swa`,SWE:`swe`,SYR:`syr`,TAM:`tam`,TEL:`tel`,TGK:`tgk`,TGL:`tgl`,THA:`tha`,TIR:`tir`,TUR:`tur`,UIG:`uig`,UKR:`ukr`,URD:`urd`,UZB:`uzb`,UZB_CYRL:`uzb_cyrl`,VIE:`vie`,YID:`yid`}})),Pn=a(((e,t)=>{t.exports={OSD_ONLY:`0`,AUTO_OSD:`1`,AUTO_ONLY:`2`,AUTO:`3`,SINGLE_COLUMN:`4`,SINGLE_BLOCK_VERT_TEXT:`5`,SINGLE_BLOCK:`6`,SINGLE_LINE:`7`,SINGLE_WORD:`8`,CIRCLE_WORD:`9`,SINGLE_CHAR:`10`,SPARSE_TEXT:`11`,SPARSE_TEXT_OSD:`12`,RAW_LINE:`13`}})),Fn=a(((e,t)=>{Kt();var n=Xt(),r=jn(),i=Mn(),a=Nn(),o=$t(),s=Pn(),{setLogging:c}=Yt();t.exports={languages:a,OEM:o,PSM:s,createScheduler:n,createWorker:r,setLogging:c,...i}}))(),K=null,q=null,In=5e3,Ln=1200,Rn=1e6;function zn(){return q||(q=(0,Fn.createWorker)(`eng+ind`).then(e=>(K=e,e)),q)}async function Bn(e){if(e.complete||await Hn(e),e.naturalWidth===0||e.naturalHeight===0)return``;let t=Un(e);if(!t)return``;let{data:n}=await(await zn()).recognize(t);return n.text}async function Vn(){K&&(await K.terminate(),K=null,q=null)}function Hn(e){return new Promise(t=>{if(e.complete){t();return}let n,r=()=>{e.removeEventListener(`load`,i),e.removeEventListener(`error`,i),n!==void 0&&window.clearTimeout(n)},i=()=>{r(),t()};e.addEventListener(`load`,i,{once:!0}),e.addEventListener(`error`,i,{once:!0}),n=window.setTimeout(i,In)})}function Un(e){try{let t=Wn(e.naturalWidth,e.naturalHeight),n=document.createElement(`canvas`);n.width=t.width,n.height=t.height;let r=n.getContext(`2d`);return r?(r.drawImage(e,0,0,t.width,t.height),r.getImageData(0,0,1,1),n):null}catch{return null}}function Wn(e,t){let n=Math.min(1,Ln/Math.max(e,t)),r=Math.min(1,Math.sqrt(Rn/Math.max(1,e*t))),i=Math.min(n,r);return{width:Math.max(1,Math.round(e*i)),height:Math.max(1,Math.round(t*i))}}var Gn=350,Kn=128,J=null,qn,Y=0,Jn=!1,X=!1,Yn=!1,Z=null,Xn=!1,Zn=B,Q=new Map;Qn();function Qn(){if(document.readyState===`loading`){document.addEventListener(`DOMContentLoaded`,$n,{once:!0});return}$n()}function $n(){if(!document.body){window.setTimeout($n,50);return}k(),ht(),nt(()=>$(0)),It(Bn),window.addEventListener(`pagehide`,()=>void Vn(),{once:!0}),gt(),j(),M(),it(e=>{Zn=e,$(0)}),er(),$(0),rt(e=>{Zn=e,$(0)})}function er(){J?.disconnect(),J=new MutationObserver(e=>{Xn||e.some(C)&&$(Gn)}),J.observe(document.body,{attributeFilter:[`src`,`srcset`,`sizes`,`data-src`,`data-srcset`],attributes:!0,characterData:!0,childList:!0,subtree:!0})}function $(e){X=!0,Y++,window.clearTimeout(qn),qn=window.setTimeout(()=>{rr()},e)}function tr(){return new Promise(e=>setTimeout(e,0))}var nr=12;async function rr(){if(!Jn){Jn=!0;try{for(;X;)X=!1,await ir(Y)}finally{Jn=!1}}}async function ir(e){if(!document.body)return;let t,n;try{t=st(`${Date.now()}-${e}`),n=new Map;let r=await sr(S(document.body),t,n,e);if(Y!==e)return;let i=ne(r);dt(i,n),ut(t,i),or(()=>{gt(),ar(i),ie(i)}),tt(t)}catch{return}_r({stats:t,keywordAlgorithmTimes:n,expectedSeq:e})}function ar(e){if(!Zn.blurTextEnabled){M();return}try{te(e)}catch{M()}}function or(e){let t=J!==null;Xn=!0,J?.disconnect();try{e()}finally{Xn=!1,t&&document.body&&er()}}async function sr(e,t,n,r){let i=[],a=performance.now();for(let o=0;o<e.length;o++){if(Y!==r)return i;let s=e[o],c=s.nodeValue??``,l=Je(c),u=$e(l.value);H(t,`AhoCorasick`,u.elapsedMs,u.comparisonCount);let d=D(c,l);H(t,`RegEx`,d.executionMs,c.length),lt(t,`RegEx`,d.matches.length),ur(i,s,d.matches,d.executionMs);for(let e of I){let o=Qe(c,e,l,u.matches,u.comparisonCount,u.elapsedMs);for(let r of o)ct(t,r,e,n,{countCost:!lr(r)});if(cr(i,s,c,e,o),performance.now()-a>=nr&&(await tr(),a=performance.now(),Y!==r))return i}if(performance.now()-a>=nr&&o<e.length-1&&(await tr(),a=performance.now(),Y!==r))return i}return i}function cr(e,t,n,r,i){for(let a of i)if(a.matched)for(let i=0;i<a.matchIndexes.length;i++){let o=a.matchIndexes[i],s=a.matchLengths?.[i]??r.length,c=Math.min(n.length,o+s);o<0||c<=o||e.push({node:t,startIndex:o,endIndex:c,text:n.slice(o,c),keyword:r,algorithm:a.algorithm,occurrences:0,executionTime:a.executionTime})}}function lr(e){return e.algorithm===`AhoCorasick`}function ur(e,t,n,r){for(let i of n)e.push({node:t,startIndex:i.start,endIndex:i.end,text:i.keyword,keyword:i.keyword,algorithm:`RegEx`,occurrences:0,executionTime:r})}async function dr(e,t,n){let r=Lt();if(!r||!document.body)return;let i=At(document.body);if(i.length===0)return;let a=!1;for(let o of i){if(Y!==n)return;let i=G(o),s=o.currentSrc||o.src;o.classList.contains(`judol-image`)&&o.dataset.judolImageKey!==i&&Wt(o);try{if(Rt({imageElement:o,extractedText:fr(o)},I,e,t)){jt(o,i),a=!0;continue}let c=await hr(o,r);if(Y!==n||(o.currentSrc||o.src)!==s)return;jt(o,G(o)),Rt({imageElement:o,extractedText:c},I,e,t)&&(a=!0)}catch{}}a&&Y===n&&tt(e)}function fr(e){let t=e.currentSrc||e.src;return[e.alt,e.title,e.getAttribute(`aria-label`)??``,pr(t),mr(e)?document.title:``].filter(Boolean).join(` `)}function pr(e){if(!e)return``;try{let t=new URL(e,window.location.href).pathname.split(`/`).filter(Boolean).at(-1)??``;return decodeURIComponent(t).replace(/\.[a-z0-9]+$/iu,``).replace(/[-_+.]+/gu,` `)}catch{return e.replace(/[-_+.]+/gu,` `)}}function mr(e){return document.body.childElementCount===1&&document.body.firstElementChild===e}function hr(e,t){let n=e.currentSrc||e.src,r=Q.get(n);if(r)return r;let i=t(e).catch(e=>{throw Q.delete(n),e});return Q.set(n,i),gr(),i}function gr(){for(;Q.size>Kn;){let e=Q.keys().next().value;if(e===void 0)return;Q.delete(e)}}function _r(e){Z=e,vr()}async function vr(){if(!Yn){Yn=!0;try{for(;Z!==null;){let e=Z;Z=null,await dr(e.stats,e.keywordAlgorithmTimes,e.expectedSeq)}}finally{Yn=!1}}}