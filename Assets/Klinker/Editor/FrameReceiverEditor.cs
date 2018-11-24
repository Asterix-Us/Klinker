using UnityEngine;
using UnityEditor;
using System.Linq;

namespace Klinker
{
    [CustomEditor(typeof(FrameReceiver))]
    public sealed class FrameReceiverEditor : Editor
    {
        SerializedProperty _deviceSelection;
        SerializedProperty _targetTexture;
        SerializedProperty _targetRenderer;
        SerializedProperty _targetMaterialProperty;

        GUIContent[] _deviceLabels;
        int[] _deviceOptions;

        readonly GUIContent _labelDevice = new GUIContent("Device");

        public override bool RequiresConstantRepaint()
        {
            return Application.isPlaying;
        }

        void OnEnable()
        {
            _deviceSelection = serializedObject.FindProperty("_deviceSelection");
            _targetTexture = serializedObject.FindProperty("_targetTexture");
            _targetRenderer = serializedObject.FindProperty("_targetRenderer");
            _targetMaterialProperty = serializedObject.FindProperty("_targetMaterialProperty");

            // Scan all available devices.
            var devices = DeviceManager.GetDeviceNames();
            _deviceLabels = devices.Select((x) => new GUIContent(x)).ToArray();
            _deviceOptions = Enumerable.Range(0, devices.Length).ToArray();
        }

        public override void OnInspectorGUI()
        {
            serializedObject.Update();

            // Device selector with format information
            EditorGUILayout.IntPopup(_deviceSelection, _deviceLabels, _deviceOptions, _labelDevice);
            EditorGUILayout.LabelField("Format", ((FrameReceiver)target).formatName);

            // Target texture/renderer
            EditorGUILayout.PropertyField(_targetTexture);
            EditorGUILayout.PropertyField(_targetRenderer);

            // Material property selector (only shown with a target renderer)
            if (_targetRenderer.objectReferenceValue != null)
            {
                EditorGUI.indentLevel++;
                MaterialPropertySelector.DropdownList(_targetRenderer, _targetMaterialProperty);
                EditorGUI.indentLevel--;
            }

            serializedObject.ApplyModifiedProperties();
        }
    }
}
