import 'dart:async';

import 'package:flutter/material.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'auth_method.dart';
import 'logs/logs_screen.dart';
import 'settings/settings_screen.dart';
import 'unlock/unlock_screen.dart';

void main() => runApp(const DoorApp());

class DoorApp extends StatefulWidget {
  const DoorApp({super.key});

  @override
  State<DoorApp> createState() => _DoorAppState();
}

class _DoorAppState extends State<DoorApp> {
  static const _methodsKey = 'enabled_auth_methods';
  final _preferences = SharedPreferencesAsync();
  int _page = 0;
  bool _settingsLoaded = false;
  final Set<AuthMethod> _enabledMethods = {AuthMethod.biometric};
  final Set<AuthMethod> _completedMethods = {};
  Timer? _timer;
  DateTime? _deadline;
  int _remainingSeconds = 60;

  @override
  void initState() {
    super.initState();
    _loadMethods();
  }

  Future<void> _loadMethods() async {
    try {
      final saved = await _preferences.getStringList(_methodsKey);
      if (!mounted) return;
      setState(() {
        if (saved != null) {
          final methods = AuthMethod.values.where(
            (method) => saved.contains(method.name),
          );
          _enabledMethods
            ..clear()
            ..addAll(methods);
          if (_enabledMethods.isEmpty) {
            _enabledMethods.add(AuthMethod.biometric);
          }
        }
        _settingsLoaded = true;
      });
    } on Exception {
      if (!mounted) return;
      setState(() => _settingsLoaded = true);
    }
  }

  Future<void> _saveMethods() async {
    try {
      await _preferences.setStringList(
        _methodsKey,
        _enabledMethods.map((method) => method.name).toList(),
      );
    } on Exception {
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Could not save settings on this phone.')),
      );
    }
  }

  void _stopTimer() {
    _timer?.cancel();
    _timer = null;
    _deadline = null;
    _remainingSeconds = 60;
  }

  void _resetProgress() {
    setState(() {
      _completedMethods.clear();
      _stopTimer();
    });
  }

  void _tick() {
    final deadline = _deadline;
    if (deadline == null) return;

    final millisecondsLeft = deadline.difference(DateTime.now()).inMilliseconds;
    if (millisecondsLeft <= 0) {
      _resetProgress();
      return;
    }

    setState(() => _remainingSeconds = (millisecondsLeft / 1000).ceil());
  }

  void _setMethod(AuthMethod method, bool enabled) {
    if (!enabled &&
        _enabledMethods.length == 1 &&
        _enabledMethods.contains(method)) {
      return;
    }

    setState(() {
      if (enabled) {
        _enabledMethods.add(method);
      } else {
        _enabledMethods.remove(method);
        _completedMethods.remove(method);
        if (_completedMethods.isEmpty) _stopTimer();
      }
    });
    unawaited(_saveMethods());
  }

  void _approveMethod(AuthMethod method) {
    if (!_enabledMethods.contains(method)) return;
    if (_deadline != null && !DateTime.now().isBefore(_deadline!)) {
      _resetProgress();
    }
    setState(() {
      if (_completedMethods.isEmpty) {
        _deadline = DateTime.now().add(const Duration(minutes: 1));
        _remainingSeconds = 60;
        _timer = Timer.periodic(const Duration(seconds: 1), (_) => _tick());
      }
      _completedMethods.add(method);
    });
  }

  @override
  void dispose() {
    _timer?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    const blue = Color(0xFF02569B);
    const accent = Color(0xFF13B9FD);
    final colors = ColorScheme.fromSeed(seedColor: blue).copyWith(
      primary: blue,
      secondary: accent,
      surface: const Color(0xFFF3F9FD),
    );

    return MaterialApp(
      title: 'Secure Door',
      theme: ThemeData(
        colorScheme: colors,
        useMaterial3: true,
        scaffoldBackgroundColor: colors.surface,
        appBarTheme: const AppBarTheme(
          backgroundColor: blue,
          foregroundColor: Colors.white,
          centerTitle: false,
        ),
        cardTheme: CardThemeData(
          color: Colors.white,
          elevation: 0,
          shape: RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(16),
            side: const BorderSide(color: Color(0xFFD9EBF7)),
          ),
        ),
        navigationBarTheme: NavigationBarThemeData(
          backgroundColor: Colors.white,
          indicatorColor: const Color(0xFFBCEEFF),
          iconTheme: WidgetStateProperty.resolveWith((states) {
            return IconThemeData(
              color: states.contains(WidgetState.selected)
                  ? blue
                  : const Color(0xFF607D91),
            );
          }),
        ),
      ),
      home: Scaffold(
        appBar: AppBar(title: const Text('Secure Door · Demo')),
        body: !_settingsLoaded
            ? const Center(child: CircularProgressIndicator())
            : switch (_page) {
                0 => const LogsScreen(),
                1 => SettingsScreen(
                  enabledMethods: _enabledMethods,
                  onMethodChanged: _setMethod,
                ),
                _ => UnlockScreen(
                  enabledMethods: _enabledMethods,
                  completedMethods: _completedMethods,
                  remainingSeconds: _remainingSeconds,
                  onApprove: _approveMethod,
                  onReset: _resetProgress,
                ),
              },
        bottomNavigationBar: NavigationBar(
          selectedIndex: _page,
          onDestinationSelected: (index) => setState(() => _page = index),
          destinations: const [
            NavigationDestination(icon: Icon(Icons.history), label: 'Logs'),
            NavigationDestination(icon: Icon(Icons.tune), label: 'Settings'),
            NavigationDestination(icon: Icon(Icons.lock_open), label: 'Unlock'),
          ],
        ),
      ),
    );
  }
}
