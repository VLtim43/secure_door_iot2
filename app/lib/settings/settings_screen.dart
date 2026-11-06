import 'package:flutter/material.dart';

import '../auth_method.dart';

class SettingsScreen extends StatelessWidget {
  const SettingsScreen({
    super.key,
    required this.enabledMethods,
    required this.onMethodChanged,
  });

  final Set<AuthMethod> enabledMethods;
  final void Function(AuthMethod, bool) onMethodChanged;

  @override
  Widget build(BuildContext context) {
    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        Text('Door settings', style: Theme.of(context).textTheme.headlineSmall),
        const SizedBox(height: 4),
        const Text(
          'Enable methods for this door. Changes are local preview only.',
        ),
        const SizedBox(height: 16),
        for (final method in AuthMethod.values)
          Card(
            child: SwitchListTile(
              secondary: Icon(
                method.icon,
                color: Theme.of(context).colorScheme.primary,
              ),
              title: Text(method.label),
              value: enabledMethods.contains(method),
              onChanged:
                  enabledMethods.length == 1 && enabledMethods.contains(method)
                  ? null
                  : (enabled) => onMethodChanged(method, enabled),
            ),
          ),
      ],
    );
  }
}
