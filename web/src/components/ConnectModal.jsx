import { useState, useEffect } from 'react';

const ConnectModal = ({ onClose, onConnect }) => {
  const [step, setStep] = useState(0);

  useEffect(() => {
    if (step === 0)      setTimeout(() => setStep(1), 800);
    else if (step === 1) setTimeout(() => setStep(2), 1200);
    else if (step === 2) setTimeout(() => setStep(3), 900);
    else if (step === 3) setTimeout(() => { onConnect(); onClose(); }, 600);
  }, [step, onConnect, onClose]);

  const steps = [
    { label:"Requesting serial port…",            done: step>0 },
    { label:"Opening USB CDC @ 115200 baud…",      done: step>1 },
    { label:"Handshaking with RP2040 bridge…",     done: step>2 },
    { label:"Server authentication…",              done: step>3 },
  ];

  return (
    <div style={{
      position:"fixed", inset:0, background:"rgba(6,8,9,0.85)", backdropFilter:"blur(8px)",
      display:"flex", alignItems:"center", justifyContent:"center", zIndex:1000
    }}>
      <div style={{
        background:"var(--bg-surface)", border:"1px solid var(--border)",
        borderRadius:"var(--r-xl)", padding:32, width:380,
        boxShadow:"0 0 60px var(--green-glow)"
      }} className="animate-in">
        <div style={{fontFamily:"var(--font-display)", fontSize:16, fontWeight:800, color:"var(--green)", marginBottom:6}}>
          Connecting Device
        </div>
        <div style={{fontSize:12, color:"var(--text-3)", marginBottom:24}}>
          Web Serial API · Chrome/Edge required
        </div>
        <div style={{display:"flex", flexDirection:"column", gap:10}}>
          {steps.map((s, i) => (
            <div key={i} style={{display:"flex", alignItems:"center", gap:10, fontFamily:"var(--font-mono)", fontSize:12}}>
              {s.done
                ? <span style={{color:"var(--green)", fontSize:14}}>✓</span>
                : i === step
                  ? <span style={{animation:"spin 0.8s linear infinite", display:"inline-block", color:"var(--amber)"}}>⟳</span>
                  : <span style={{color:"var(--text-3)"}}>○</span>
              }
              <span style={{color: s.done ? "var(--text-1)" : i === step ? "var(--amber)" : "var(--text-3)"}}>{s.label}</span>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
};

export default ConnectModal;
