import { useState, useEffect } from 'react';
import Sidebar from './components/Sidebar';
import StatusBar from './components/StatusBar';
import ConnectModal from './components/ConnectModal';
import { Badge } from './components/ui';
import Dashboard from './views/Dashboard';
import ChipDatabase from './views/ChipDatabase';
import Operations from './views/Operations';
import HexEditor from './views/HexEditor';
import ProtocolAnalyzer from './views/ProtocolAnalyzer';
import Settings from './views/Settings';
import FirmwareUpdate from './views/FirmwareUpdate';

export default function App() {
  const [view, setView] = useState("dashboard");
  const [connected, setConnected] = useState(false);
  const [showModal, setShowModal] = useState(false);
  const [selectedChip, setSelectedChip] = useState(null);
  const [serverOk] = useState(true);

  const deviceInfo = { port:"COM4 · USB (RP2040)", fw:"2.4.1", speed:"12 Mbps" };

  const handleSelectChip = (chip) => {
    setSelectedChip(chip);
    setView("operation");
  };

  const handleNavChange = (v) => {
    if (connected || v === "dashboard") setView(v);
  };

  return (
    <>
      <div className="scanline" />
      {showModal && <ConnectModal onClose={()=>setShowModal(false)} onConnect={()=>setConnected(true)} />}
      <div style={{display:"flex", flexDirection:"column", height:"100vh", overflow:"hidden"}}>
        {/* Top bar */}
        <div style={{
          display:"flex", alignItems:"center", gap:0,
          height:48, background:"var(--bg-base)", borderBottom:"1px solid var(--border-dim)",
          flexShrink:0, paddingLeft:57,
        }}>
          <div style={{
            display:"flex", alignItems:"center", gap:10, padding:"0 20px",
            borderRight:"1px solid var(--border-dim)", height:"100%"
          }}>
            <span style={{fontFamily:"var(--font-display)", fontSize:14, fontWeight:800, color:"var(--green)", letterSpacing:"0.04em"}}>
              UNIPROG
            </span>
            <Badge small>SaaS v2.4</Badge>
          </div>
          <StatusBar
            connected={connected} deviceInfo={deviceInfo} serverOk={serverOk}
            onConnect={()=>setShowModal(true)} onDisconnect={()=>setConnected(false)}
          />
        </div>

        <div style={{display:"flex", flex:1, overflow:"hidden"}}>
          <Sidebar active={view} onChange={handleNavChange} connected={connected} />
          <div style={{flex:1, overflow:"hidden", animation:"fadeIn 0.2s ease"}} key={view}>
            {view==="dashboard"   && <Dashboard connected={connected} onConnect={()=>setShowModal(true)} chipSelected={selectedChip} onSelectChip={handleSelectChip} />}
            {view==="chips"       && <ChipDatabase onSelectChip={handleSelectChip} selectedChip={selectedChip} />}
            {view==="operation"   && <Operations chip={selectedChip} connected={connected} />}
            {view==="hexeditor"   && <HexEditor chip={selectedChip} />}
            {view==="analyzer"    && <ProtocolAnalyzer />}
            {view==="settings"    && <Settings />}
            {view==="firmware"    && <FirmwareUpdate connected={connected} />}
          </div>
        </div>
      </div>
    </>
  );
}
