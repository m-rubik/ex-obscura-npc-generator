import { useEffect } from 'react';
import { useCallback, useMemo, useState } from 'react';
import ReactFlow, {
  Background,
  Controls,
  MiniMap,
  Panel,
  useNodesState,
  useEdgesState,
  addEdge,
  Connection,
  Edge,
  Node,
  NodeTypes,
} from 'reactflow';
import 'reactflow/dist/style.css';
import { NPCNode } from './nodes/NPCNode';
import { BuildingNode } from './nodes/BuildingNode';
import { fetchWorld, generateNPC } from './api';
import { NPC, relationshipColors } from './types';
import './App.css';

const nodeTypes: NodeTypes = {
  npc: NPCNode,
  building: BuildingNode,
};

function App() {
  const [nodes, setNodes, onNodesChange] = useNodesState([]);
  const [edges, setEdges, onEdgesChange] = useEdgesState([]);
  const [selectedNode, setSelectedNode] = useState<Node | null>(null);
  const [loading, setLoading] = useState(true);
  const [npcs, setNpcs] = useState<NPC[]>([]);

  // Load initial world from backend
  useEffect(() => {
    const loadWorld = async () => {
      setLoading(true);
      try {
        const worldData = await fetchWorld();
        setNpcs(worldData.npcs);

        // Convert NPCs to React Flow nodes
        const newNodes: Node[] = worldData.npcs.map((npc, idx) => ({
          id: npc.id,
          type: 'npc',
          position: { x: idx * 280, y: 50 },
          data: { npc },
        }));

        // Add a few building nodes
        const buildings = [
          { id: 'building-1', name: 'Old Bank', type: 'Financial' },
          { id: 'building-2', name: 'Town Hall', type: 'Government' },
        ];

        buildings.forEach((building, idx) => {
          newNodes.push({
            id: building.id,
            type: 'building',
            position: { x: idx * 280, y: 250 },
            data: { name: building.name, type: building.type },
          });
        });

        setNodes(newNodes);

        // Create edges from relationships
        const newEdges: Edge[] = (worldData.relationships || []).map(
          (rel: any, idx: number) => {
            const color = relationshipColors[rel.type] || relationshipColors.unknown;
            return {
              id: `edge-${idx}`,
              source: rel.from,
              target: rel.to,
              label: rel.type,
              style: { stroke: color, strokeWidth: 2 },
              animated: true,
              labelStyle: { fill: color, fontSize: '11px' },
            };
          }
        );

        setEdges(newEdges);
      } catch (err) {
        console.error('Failed to load world:', err);
      } finally {
        setLoading(false);
      }
    };

    loadWorld();
  }, [setNodes, setEdges]);

  const onConnect = useCallback(
    (connection: Connection) => setEdges((eds) => addEdge(connection, eds)),
    [setEdges]
  );

  const onSelectionChange = useCallback(
    ({ nodes }: { nodes: Node[] }) => {
      setSelectedNode(nodes.length > 0 ? nodes[0] : null);
    },
    []
  );

  const boardSummary = useMemo(
    () => ({
      nodes: nodes.length,
      edges: edges.length,
    }),
    [nodes.length, edges.length]
  );

  const generateMore = async () => {
    setLoading(true);
    try {
      const newNpc = await generateNPC();
      setNpcs((prev) => [...prev, newNpc]);
      const nodeIndex = nodes.length;
      const newNode = {
        id: `npc-${nodeIndex}`,
        type: 'npc',
        position: { x: nodeIndex * 250, y: 50 },
        data: { npc: newNpc },
      };
      setNodes((prev) => [...prev, newNode]);
    } catch (err) {
      console.error('Failed to generate NPC:', err);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="app-shell">
      <aside className="sidebar">
        <h1>Ex Obscura</h1>
        <p>Investigation board for generated mysteries.</p>
        <div className="stats">
          <div>Nodes: {boardSummary.nodes}</div>
          <div>Edges: {boardSummary.edges}</div>
        </div>
        <button
          className="generate-btn"
          onClick={generateMore}
          disabled={loading}
        >
          {loading ? 'Generating...' : 'Add NPC'}
        </button>
        <div className="details">
          <h2>Selected</h2>
          {selectedNode ? (
            <>
              <div className="detail-row">ID: {selectedNode.id}</div>
              <div className="detail-row">Type: {selectedNode.type}</div>
            </>
          ) : (
            <div className="detail-row">No node selected</div>
          )}
        </div>
      </aside>

      <main className="canvas-panel">
        <ReactFlow
          nodes={nodes}
          edges={edges}
          onNodesChange={onNodesChange}
          onEdgesChange={onEdgesChange}
          onConnect={onConnect}
          onSelectionChange={onSelectionChange}
          nodeTypes={nodeTypes}
          fitView
        >
          <Background gap={16} />
          <MiniMap />
          <Controls />
          <Panel position="top-right">
            <div className="panel-note">
              {loading ? 'Loading...' : 'Drag nodes, connect relationships'}
            </div>
          </Panel>
        </ReactFlow>
      </main>
    </div>
  );
}

export default App;
