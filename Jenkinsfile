pipeline {
  agent any
  stages {
    stage('Checkout') { steps { checkout scm } }
    stage('Configure') { steps { sh 'cmake -S . -B build -DTASKFORGE_BUILD_TESTS=ON' } }
    stage('Build') { steps { sh 'cmake --build build -j2' } }
    stage('Unit Tests') { steps { sh 'ctest --test-dir build --output-on-failure' } }
    stage('Test Report') { steps { junit allowEmptyResults: true, testResults: 'build/**/*.xml' } }
    stage('Docker Build') { steps { sh 'docker build -f docker/Dockerfile.gateway -t taskforge/gateway:ci .' } }
  }
}
