const expressionEl=document.getElementById('expression'),resultEl=document.getElementById('result'),historyList=document.getElementById('historyList');
let expression='',degrees=true,history=[];
function render(){expressionEl.textContent=expression||'0';}
function insert(v){expression+=v;render();resultEl.textContent='Ready';}
function showError(msg){resultEl.textContent=msg;}
async function calculate(){
 if(!expression.trim())return;
 try{
  const res=await fetch('/api/calculate',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({expression,degrees})});
  const data=await res.json();
  if(!res.ok||!data.ok)throw new Error(data.error||'Calculation failed');
  const answer=String(data.result);resultEl.textContent=answer;
  history.unshift({expression,answer});history=history.slice(0,8);renderHistory();
 }catch(e){showError(e.message);}
}
function renderHistory(){historyList.innerHTML='';if(!history.length){historyList.innerHTML='<li class="empty">No calculations yet.</li>';return;}
 history.forEach(item=>{const li=document.createElement('li');const left=document.createElement('span');left.textContent=item.expression+' =';const right=document.createElement('strong');right.textContent=item.answer;li.append(left,right);historyList.append(li);});}
document.querySelectorAll('[data-insert]').forEach(b=>b.addEventListener('click',()=>insert(b.dataset.insert)));
document.querySelectorAll('[data-action]').forEach(b=>b.addEventListener('click',()=>{const a=b.dataset.action;if(a==='clear'){expression='';render();resultEl.textContent='Ready';}if(a==='back'){expression=expression.slice(0,-1);render();}if(a==='equals')calculate();}));
document.getElementById('angle').addEventListener('click',e=>{degrees=!degrees;e.target.textContent=degrees?'DEG':'RAD';});
document.getElementById('theme').addEventListener('click',()=>document.body.classList.toggle('light'));
document.getElementById('clearHistory').addEventListener('click',()=>{history=[];renderHistory();});
document.addEventListener('keydown',e=>{if(/^[0-9.]$/.test(e.key))insert(e.key);else if(['+','-','*','/','(',')','^','%'].includes(e.key))insert(e.key);else if(e.key==='Enter'){e.preventDefault();calculate();}else if(e.key==='Backspace'){expression=expression.slice(0,-1);render();}else if(e.key==='Escape'){expression='';render();resultEl.textContent='Ready';}});
