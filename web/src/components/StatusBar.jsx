import { Badge, Dot, Btn } from './ui';

const StatusBar = ({ connected, deviceInfo, serverOk, onConnect, onDisconnect }) => (
  <div style={{
    display:"flex", alignItems:"center", gap:16, padding:"0 20px",
    height:40, borderBottom:"1px solid var(--border-dim)",
    background:"var(--bg-base)", flexShrink:0, flex:1,
  }}>
    <div style={{display:"flex", alignItems:"center", gap:6}}>
      <Dot color={connected?"var(--green)":"var(--red)"} pulse={connected} />
      <span style={{fontFamily:"var(--font-mono)", fontSize:11, color:"var(--text-2)"}}>
        {connected ? deviceInfo.port : "NOT CONNECTED"}
      </span>
    </div>
    {connected && <>
      <div style={{width:1, height:16, background:"var(--border-dim)"}} />
      <span style={{fontFamily:"var(--font-mono)", fontSize:10, color:"var(--text-3)"}}>
        RP2040 Bridge v{deviceInfo.fw} · {deviceInfo.speed}
      </span>
      <div style={{width:1, height:16, background:"var(--border-dim)"}} />
      <Badge color="var(--cyan)" small>USB CDC</Badge>
      <Badge color={serverOk?"var(--green)":"var(--amber)"} small>{serverOk?"SERVER OK":"OFFLINE"}</Badge>
    </>}
    <div style={{marginLeft:"auto", display:"flex", gap:8, alignItems:"center"}}>
      <span style={{fontFamily:"var(--font-mono)", fontSize:10, color:"var(--text-3)"}}>
        {new Date().toLocaleTimeString()}
      </span>
      {connected
        ? <Btn variant="danger" small onClick={onDisconnect}>⏏ Disconnect</Btn>
        : <Btn variant="primary" small onClick={onConnect} icon="⚡">Connect Device</Btn>
      }
    </div>
  </div>
);

export default StatusBar;
